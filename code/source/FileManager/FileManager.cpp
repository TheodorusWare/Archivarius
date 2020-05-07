/*
Copyright (C) 2018-2020 Theodorus Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <FileManager/FileManagerDef.h>
#include <FileManager/FileManager.h>
#include <FileManager/EncodeSettings.h>
#include <FileManager/DecodeSettings.h>
#include <FileManager/ArchiveInfo.h>
#include <FileManager/ArchiverAbout.h>
#include <FileManager/ArchiverOptions.h>
#include <FileManager/StringWidget.h>
#include <Common/StringLib.h>
#include <Common/FileAccess.h>
#include <Common/Registry.h>
#include <Common/ConvertBytes.h>
#include <Common/TextStream.h>
#include <Compress/Archive.h>
#include <Widget/Event.h>
#include <Widget/Button.h>
#include <Widget/Menu.h>
#include <time.h>

namespace tas
{

FileManager* FileManager::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();
String& getLogName();

ArchiverSettings::ArchiverSettings()
{
	init();
}

void ArchiverSettings::init()
{
	forn(15) operator[](i) = -1;
	cryptFile = 0;
	showPass = 0;
	filePath = 1;
	windowMaximize = 0;
	windowAlign = 0;
	fullRowSelect = 1;
	sortType = 0;
	sortMode = 1;
}

byte& ArchiverSettings::operator[](int i)
{
	return *(&level + i);
}

FileManager::FileManager()
{
	mPtr = this;
	mLocalisationId = "en";
	mFileSystemType = 1;
	mFileSystemLevel = 0;
	mArchiveLevel = 0;
	mAddressItemCount = 0;
	mDriveCount = 0;
	mButtonUp = 0;
	mUnpackType = 0;
	mAppendType = 0;
	mListEdit = 0;
	mApplication = 0;
	mWindow = 0;
	mToolbar = 0;
	mStatusbar = 0;
	mgAddress = 0;
	mgFileList = 0;
	mItemString = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(600, 400);
	mButtonSize = Vector2i(70, 25);
	mBuffer.reserve(BUFFER_SIZE);
	mBufferw.reserve(BUFFER_SIZE);
	mAddressCurrent.reserve(BUFFER_SIZE);
	mAddressArchive.reserve(BUFFER_SIZE);
	mAddressEdit.reserve(BUFFER_SIZE);
	mAddressUp.reserve(BUFFER_SIZE);
	mPassword.reserve(BUFFER_SIZE);
	mArchiveDirectory.reserve(BUFFER_SIZE);
	mArchiveName.reserve(BUFFER_SIZE);
	mArchiveBackup.reserve(BUFFER_SIZE);
	mDirectoryUnpack.reserve(BUFFER_SIZE);
	mFiles.reserve(64);
	mFolders.reserve(64);
	mSearch.hashEnable = 1;
	mListIndex = 0;
	mItemIndex = 0;
	mItemType = 0;
	mSortType = 0;
	mSortMode = 1;
	mSortList = 0;
	mFontSystem = 0;
	mFontLarge = 0;
	mImageLogo = 0;
	mArchiveFileList = 0;
	mTimeUpd = 0;
	mRootHash = 0;
	mSystemSegregation = 0;
	mStop = 0;
	forn(3)
	mListSelect[i] = 0;
	forn(2)
	mAddressTime[i] = 0;
}

FileManager::~FileManager()
{
	readRegistry(1);
	if(mDirectoryUnpack.length())
		removeFileShell(mDirectoryUnpack);
	safe_delete(mWindow);
	safe_delete(mApplication);
	mPtr = 0;
}

bool FileManager::create(const String& archiveFile)
{
	mAddressArchive = archiveFile;

	// registry
	bool readReg = readRegistry(0);

	// localisation
	readLocalisation();

	mApplication = new GuiApplication;

	mWindow = new Window(0, PROGRAM_TITLE, 1, mWindowSize, mWindowPos,
	                     WidgetCallbackImpl, IdleCallbackImpl);

	mWindow->setSizeMin(Vector2i(200, 250));

	uint windowProps = 1;
	if(mArchiverSettings.windowAlign or !readReg)
		windowProps |= 8; // align center
	if(mArchiverSettings.windowMaximize)
		windowProps |= 2; // maximize

	// create toolbar
	createToolbar();

	// create statusbar
	mStatusbar = new Statusbar(mWindow, ID_WC_STATUSBAR);

	// create address
	createAddress();

	// create file list
	mFileListPos = Vector2i(0, 91);
	mFileListSize = Vector2i(mWindowSize.x, mWindowSize.y - mFileListPos.y);
	mgFileList = new Listview(mWindow, ID_WC_FILELIST, mFileListSize, mFileListPos);

	// create main window
	mWindow->createEx(windowProps, 1);

	// initialise widgets
	initialise();

	// main loop
	mWindow->run();

	return 1;
}

bool FileManager::initialise()
{
	// drag & drop
	mWindow->setDragDrop(1);

	// system font
	mFontSystem = GetStockObject(DEFAULT_GUI_FONT);

	// large font
	mFontLarge = WT_APP->createFont("Segoe UI", 12);

	// create menu
	createMenu();

	// init statusbar
	int parts[] = {240, 340, -1};
	mStatusbar->setParts(parts, 3);

	// init address
	findLogicalDrives();

	// init file list
	initFileList();

	// image logo
	mImageLogo = Imagelist::loadImage(7);

	mKeyboard.setKeyState(VK_RETURN, 1);

	return 1;
}

bool FileManager::createMenu()
{
	Menu mainMenu;
	Menu fileMenu;
	Menu comdMenu;
	Menu viewMenu;
	Menu helpMenu;

	mainMenu.create(0);
	fileMenu.create(1);
	comdMenu.create(1);
	viewMenu.create(1);
	helpMenu.create(1);

	// file
	mainMenu.appendMenu(&fileMenu,      mLocalisation[0].p());
	mBuffer.format("%s\tCtrl+O", mLocalisation[1].p());
	fileMenu.appendItem(ID_MF_OPEN,     mBuffer);
	mBuffer.format("%s\tCtrl+W", mLocalisation[2].p());
	fileMenu.appendItem(ID_MF_CLOSE,    mBuffer);
	mBuffer.format("%s\tCtrl+L", mLocalisation[3].p());
	fileMenu.appendItem(ID_MF_OPEN_LOG, mBuffer);
	mBuffer.format("%s\tF11", mLocalisation[4].p());
	fileMenu.appendItem(ID_MF_OPTIONS,  mBuffer);
	fileMenu.appendSeparator();
	mBuffer.format("%s\tF12", mLocalisation[5].p());
	fileMenu.appendItem(ID_MF_EXIT,     mBuffer);

	// commands
	mainMenu.appendMenu(&comdMenu,  mLocalisation[6].p());
	mBuffer.format("%s\tF5", mLocalisation[7].p());
	comdMenu.appendItem(ID_APPEND,  mBuffer);
	mBuffer.format("%s\tF6", mLocalisation[8].p());
	comdMenu.appendItem(ID_EXTRACT, mBuffer);
	mBuffer.format("%s\tF7", mLocalisation[9].p());
	comdMenu.appendItem(ID_TEST,    mBuffer);
	mBuffer.format("%s\tF8", mLocalisation[10].p());
	comdMenu.appendItem(ID_INFO,    mBuffer);
	mBuffer.format("%s\tDel", mLocalisation[11].p());
	comdMenu.appendItem(ID_DELETE,  mBuffer);
	comdMenu.appendSeparator();
	mBuffer.format("%s\tF2", mLocalisation[12].p());
	comdMenu.appendItem(ID_MC_RENAME, mBuffer);
	mBuffer.format("%s\tF3", mLocalisation[13].p());
	comdMenu.appendItem(ID_MC_UPDATE, mBuffer);
	mBuffer.format("%s\tF4", mLocalisation[14].p());
	comdMenu.appendItem(ID_MC_SELADR, mBuffer);
	mBuffer.format("%s\tCtrl+A", mLocalisation[15].p());
	comdMenu.appendItem(ID_MC_SELALL, mBuffer);
	mBuffer.format("%s\tCtrl+C", mLocalisation[16].p());
	comdMenu.appendItem(ID_MC_COPY,   mBuffer);
	mBuffer.format("%s\tCtrl+V", mLocalisation[17].p());
	comdMenu.appendItem(ID_MC_PASTE,  mBuffer);
	mBuffer.format("%s\tCtrl+F7", mLocalisation[18].p());
	comdMenu.appendItem(ID_MC_NEWDIR, mBuffer);

	// view
	mainMenu.appendMenu(&viewMenu,  mLocalisation[19].p());
	mBuffer.format("%s\tCtrl+F3", mLocalisation[20].p());
	viewMenu.appendItem(ID_MV_NAME, mBuffer);
	mBuffer.format("%s\tCtrl+F4", mLocalisation[21].p());
	viewMenu.appendItem(ID_MV_TYPE, mBuffer);
	mBuffer.format("%s\tCtrl+F5", mLocalisation[22].p());
	viewMenu.appendItem(ID_MV_TIME, mBuffer);
	mBuffer.format("%s\tCtrl+F6", mLocalisation[23].p());
	viewMenu.appendItem(ID_MV_SIZE, mBuffer);

	// help
	mainMenu.appendMenu(&helpMenu,    mLocalisation[24].p());
	mBuffer.format("%s\tF1", mLocalisation[25].p());
	helpMenu.appendItem(ID_MH_MANUAL, mBuffer);
	helpMenu.appendItem(ID_MH_ABOUT,  mLocalisation[26].p());

	mainMenu.attach(mWindow);

	return 1;
}

bool FileManager::createToolbar()
{
	mToolbarImage.create(Vector2i(32), 5);

	// icons for toolbar buttons
	forn(5) mToolbarImage.loadIcon(2 + i);

	mToolbar = new Toolbar(mWindow, ID_WC_TOOLBAR, Vector2i(95, 60), 1, &mToolbarImage);
	mToolbar->appendButton(new ToolbarButton(ID_APPEND,  0, mLocalisation[7].p(),  mLocalisation[27].p()));
	mToolbar->appendButton(new ToolbarButton(ID_EXTRACT, 1, mLocalisation[8].p(),  mLocalisation[28].p()));
	mToolbar->appendButton(new ToolbarButton(ID_TEST,    2, mLocalisation[9].p(),  mLocalisation[29].p()));
	mToolbar->appendButton(new ToolbarButton(ID_INFO,    3, mLocalisation[10].p(), mLocalisation[30].p()));
	mToolbar->appendButton(new ToolbarButton(ID_DELETE,  4, mLocalisation[11].p(), mLocalisation[31].p()));

	return 1;
}

bool FileManager::createAddress()
{
	mAddressPos = Vector2i(0, 65);
	mAddressSize = Vector2i(mWindowSize.x, 200);
	mgAddress = new Combobox(mWindow, "state", ID_WC_ADDRESS, 7, mAddressSize, mAddressPos);
	return 1;
}

bool FileManager::findLogicalDrives()
{
	uint drives = GetLogicalDrives();
	uint mask = 1;

	mAddressImage.create(Vector2i(16), 10);
	mgAddress->setImagelist(&mAddressImage);

	byte item = 0;
	forn(16)
	{
		if(drives & mask)
		{
			mBuffer.format("%c:", 0x41 + i);
			mgAddress->setItem(mBuffer, item, item++, 0, 0);
			mAddressImage.loadFileIcon(mBuffer, 1);
		}
		mask <<= 1;
	}

	mAddressItemCount  = item;
	mDriveCount = item;

	// directory icon
	mAddressImage.loadFileIcon("/", 1);
	// archive icon
	mAddressImage.loadIcon(1);
	// fill address panel
	mgAddress->setCurrentItem(0);

	if(!folderExist(mAddressCurrent))
	{
		char address[3] = {0x43, 0x3A, 0};
		mAddressCurrent = address;
	}

	return 1;
}

bool FileManager::initFileList()
{
	// Name Size Type Modified
	mgFileList->setColumn(mLocalisation[20].p(), 0, 200, 0, 0);
	mgFileList->setColumn(mLocalisation[23].p(), 1, 100, 1, 0);
	mgFileList->setColumn(mLocalisation[21].p(), 2, 100, 0, 0);
	mgFileList->setColumn(mLocalisation[32].p(), 3, 115, 0, 0);

	int style = 1;
	if(mArchiverSettings.fullRowSelect)
		style = 3;
	mgFileList->setExtendedStyle(style);

	mListIndex = 0;

	// cmd argument
	mReturn = 0;
	if(mAddressArchive.length())
	{
		mAddressEdit = mAddressArchive;
		mReturn = addressEdited();
	}
	if(!mReturn)
	{
		mAddressEdit = mAddressCurrent;
		addressEdited();
		if(mAddressCurrent.length() == 2)
			mgAddress->setCurrentItem(mAddressCurrent[0] - 0x43);
	}

	mgFileList->setFocus();
	mgFileList->selectItem(0);
	updateStatusbar();
	return 1;
}

bool FileManager::sortFileList()
{
	_printf("sortFileList type %u mode %u\n", mSortType, mSortMode);
	// file sys, sort by crc, set sort by name
	if(mFileSystemType == 1 and mSortType == 4)
		mSortType = 0;

	uint res = 0;
	uint nSize = 0;

	// sort by type, extensions, files
	if(mSortType == 2 and mFiles.size())
	{
		byte haveExt = 1; // have ext
		// list unique extensions
		StringArray listExt;
		String activeExt;
		forn(mFiles.size())
		{
			// .[ext]
			byte unique = 1;
			uint p = -1;
			p = mFiles[i].findr(0x2E);
			if(p != MAX_UINT32)
				p++;
			else
			{
				haveExt = 0;
				continue;
			}
			activeExt = mFiles[i].substr(p);
			activeExt.tolower();
			form(u, listExt.size())
			{
				// without case sensivity
				if(activeExt == listExt[u])
				{
					unique = 0;
					break;
				}
			}
			if(unique)
				listExt.push_back(activeExt);
		}

		// sort unique list
		listExt.sortStringMode(mSortMode);

		// build list files by extensions, sort
		StringArray filesExt, filesOut;
		uint indexn = 0;
		uint listn = listExt.size();
		if(!haveExt)
			listn++;
		forn(listn)
		{
			byte withExt = 1; // with ext
			if(!haveExt)
			{
				if(mSortMode and i == 0)
					withExt = 0;
				if(!mSortMode and i == listn-1)
					withExt = 0;
			}

			// build files list by extensions
			filesExt.resize(0);
			form(u, mFiles.size())
			{
				uint p = mFiles[u].findr(0x2E);
				if(p != MAX_UINT32) // find pos
				{
					if(!withExt) continue;
					p++;
					mBuffer = mFiles[u];
					mBuffer.tolower();
					if(listExt[indexn].compare(mBuffer, p, -1) != 0) continue;
				}
				else if(withExt) continue;
				filesExt.push_back(mFiles[u]);
			}

			// sort files extensions [without case] ###
			// filesExt.sortStringMode(mSortMode);
			uint res = 0;
			for(uint i = 0; i < filesExt.size(); i++)
			{
				res = i;
				for(uint u = i+1; u < filesExt.size(); u++)
				{
					mBuffer = filesExt[res];
					mBufferw = filesExt[u];
					mBuffer.tolower();
					mBufferw.tolower();
					int cmpi = mBuffer.compare(mBufferw);
					if(mSortMode ? cmpi > 0 : cmpi < 0)
						res = u;
				}
				if(i != res)
					filesExt[i].swap(filesExt[res]);
			}

			form(u, filesExt.size())
			filesOut.push_back(filesExt[u]);
			if(withExt) indexn++;
		}

		mFiles.resize(0);
		forn(filesOut.size())
		mFiles.move_back(filesOut[i]);

		// archive files
		if(mFileSystemType == 2)
		{
			Array<ArchiveFileList*> archiveListCurrent;
			forn(mFiles.size())
			{
				form(j, mArchiveListCurrent.size())
				{
					if(mFiles[i] == mArchiveListCurrent[j]->name.substrx(0x5C, 1, 1))
					{
						archiveListCurrent.push_back(mArchiveListCurrent[j]);
						break;
					}
				}
			}
			mArchiveListCurrent = archiveListCurrent;
			updateFileListArchive();
		}
	}

	// folders, files
	form(q, 2)
	{
		// size dirs
		if(mSortType == 1 and !q) continue;
		// modified time, crc dirs in archive
		if(mSortType >= 3 and mFileSystemType == 2 and !q) continue;
		// type files
		if(mSortType == 2 and q) continue;

		nSize = !q ? mFolders.size() : mFiles.size();
		StringArray& mFilesR = !q ? mFolders : mFiles;

		for(uint i = 0; i < nSize; i++)
		{
			res = i;
			for(uint u = i + 1; u < nSize; u++)
			{
				if(mSortType == 0 or mSortType == 2) // name type
				{
					mBuffer = mFilesR[res];
					mBufferw = mFilesR[u];
					mBuffer.tolower();
					mBufferw.tolower();
					int cmpi = mBuffer.compare(mBufferw);
					if(mSortMode ? cmpi > 0 : cmpi < 0)
						res = u;
				}
				else if(mSortType == 1 or mSortType == 4) // size crc
				{
					wint sizeA, sizeB;
					if(mFileSystemType == 1)
					{
						mBuffer.format("%s\\%s", mAddressCurrent.p(), mFilesR[res].p());
						sizeA = fileSize(mBuffer);
						mBuffer.format("%s\\%s", mAddressCurrent.p(), mFilesR[u].p());
						sizeB = fileSize(mBuffer);
					}
					else
					{
						if(mSortType == 1) // size
						{
							sizeA = mArchiveListCurrent[res]->size;
							sizeB = mArchiveListCurrent[u]->size;
						}
						else if(mSortType == 4) // crc
						{
							sizeA = mArchiveListCurrent[res]->crc;
							sizeB = mArchiveListCurrent[u]->crc;
						}
					}
					if(mSortMode ? sizeA > sizeB : sizeA < sizeB) // up a > b
						res = u;
				}
				else if(mSortType == 3) // modified time
				{
					wint timeA, timeB;
					if(mFileSystemType == 1)
					{
						uint times[2];
						mBuffer.format("%s\\%s", mAddressCurrent.p(), mFilesR[res].p());
						fileTimeValue(mBuffer.p(), times, 2);
						timeA = (u64)times[1] << 32 | times[0];
						mBuffer.format("%s\\%s", mAddressCurrent.p(), mFilesR[u].p());
						fileTimeValue(mBuffer.p(), times, 2);
						timeB = (u64)times[1] << 32 | times[0];
					}
					else
					{
						uint* times = mArchiveListCurrent[res]->time;
						timeA = (u64)times[1] << 32 | times[0];
						times = mArchiveListCurrent[u]->time;
						timeB = (u64)times[1] << 32 | times[0];
					}
					if(mSortMode ? timeA > timeB : timeA < timeB) // up a > b
						res = u;
				}
			}
			if(i != res)
			{
				mFilesR[i].swap(mFilesR[res]);
				if(mFileSystemType == 2 and q) // files
				{
					ArchiveFileList* in = mArchiveListCurrent[i];
					mArchiveListCurrent[i] = mArchiveListCurrent[res];
					mArchiveListCurrent[res] = in;
				}
			}
		}
	}

	mgFileList->showArrow(mSortType, mSortMode + 1);
	return 1;
}

bool FileManager::updateFileList()
{
	mFileSystemLevel = 0;
	forn(mAddressCurrent.length())
	{
		if(mAddressCurrent[i] == 0x5C)
			mFileSystemLevel++;
	}
	mButtonUp = mFileSystemLevel != 0;

	if(mFileSystemType == 2)
	{
		mArchiveLevel = 0;
		forn(mAddressArchive.length())
		{
			if(mAddressArchive[i] == 0x5C) // '\'
				mArchiveLevel++;
		}
	}

	mReturn = 0;
	mListIndex = mgFileList->getSelectItem(-1);

	if(!mSortList)
	{
		mFiles.resize(0);
		mFolders.resize(0);
		if(mFileSystemType == 1)
			mReturn = mSearch.find(mAddressCurrent, mFiles, mFolders);
		else
			mReturn = searchArchiveFiles();
	}

	sortFileList();
	mFileListImage.destroy();
	mFileListImage.create(Vector2i(16), mFiles.size() + 4);
	mgFileList->setImagelist(&mFileListImage);

	// icons up dir
	mFileListImage.loadIcon("explorer.exe", 101);
	mFileListImage.loadFileIcon("//", 1);

	// delete items
	mgFileList->deleteAllItems();

	/*
		1. insert folders
		2. insert mFiles
	*/

	mBuffer.clear();
	uint foldersCount = mFolders.size();
	if(mButtonUp)
		foldersCount++;

	forn(foldersCount)
	{
		uint iconId = 0;
		uint it = mButtonUp ? i-1 : i;
		mgFileList->insertItem(i);
		if(!i and mButtonUp)
			mBuffer = "..";
		else
		{
			mBuffer = mFolders[it];
			iconId = 1;
		}
		mgFileList->setItem(mBuffer, i, 0, iconId);

		// type
		if(!i and mButtonUp)
			mBuffer = mLocalisation[121]; // "Up";
		else
			mBuffer = mLocalisation[105]; // "Directory"
		mgFileList->setItem(mBuffer, i, 2, 0);

		if((!i and mButtonUp) or mFileSystemType == 2) continue;

		// last modified time
		mBuffer.format("%s\\%s", mAddressCurrent.p(), mFolders[it].p());
		mReturn = fileTime(mBuffer, mBuffer, 2);
		mgFileList->setItem(mBuffer, i, 3, 0);
	}

	ArchiveFileList* list = 0;

	forn(mFiles.size())
	{
		mBuffer.format("%s\\%s", mAddressCurrent.p(), mFiles[i].p());
		mReturn = mFileListImage.loadFileIcon(mBuffer, 0);
		assert(mReturn);

		if(mFileSystemType == 2)
			list = mArchiveListCurrent[i];

		// file name
		uint it = mFolders.size() + i;
		if(mButtonUp) it++;
		mgFileList->insertItem(it);
		mgFileList->setItem(mFiles[i], it, 0, i + 2);

		// file size
		wint size = 0;

		// last modified time
		if(mFileSystemType == 1)
		{
			size = fileSize(mBuffer);
			mReturn = fileTime(mBuffer, mBuffer, 2);
			mgFileList->setItem(mBuffer, it, 3, 0);
		}
		else
		{
			fileTimeString(list->time, mBuffer);
			mgFileList->setItem(mBuffer, it, 3, 0);
			// crc-32
			mBuffer.format("%X", list->crc);
			mgFileList->setItem(mBuffer, it, 4, 0);
			size = list->size;
		}

		// 12389 -> 12 389
		convertDecimalSpace(&mBuffer, size);
		mgFileList->setItem(mBuffer, it, 1, 0);

		// type
		getFileType(mFiles[i], mBuffer);
		mgFileList->setItem(mBuffer, it, 2, 0);
	}

	// last modified time
	if(mFileSystemType == 1)
	{
		if(mFileSystemLevel != 0)
		{
			mReturn = fileTimeValue(mAddressCurrent, mAddressTime, 2);
		}
		else
		{
			mRootHash = mSearch.hash;
		}
	}
	if(mSortList)
		mgFileList->selectItem(mListIndex);
	mSortList = 0;
	return 1;
}

int FileManager::getListSelect()
{
	mItemString = 0;
	mListIndex = mgFileList->getSelectItem(mListIndex);
	if(mListIndex == -1)
		return mItemType = 0; // stop
	mItemIndex = mListIndex;
	if(mButtonUp)
	{
		if(mItemIndex == 0)
			return mItemType = 3;
		mItemIndex--;
	}
	if(mItemIndex < mFolders.size())
	{
		mItemString = mFolders[mItemIndex].p();
		return mItemType = 1;
	}
	mItemIndex -= mFolders.size();
	mItemString = mFiles[mItemIndex].p();
	return mItemType = 2;
}

bool FileManager::updateStatusbar()
{
	forn(3)
	mListSelect[i] = 0;
	mListIndex = -1;
	while(getListSelect())
	{
		if(mItemType == 3) continue;
		assert(mItemType);
		mListSelect[mItemType - 1]++;
	}
	mListSelect[2] = mListSelect[0] + mListSelect[1]; // selected count
	// Selected %u / %u directories %u / %u files
	// %ls %u / %u %ls %u / %u %ls
	mBuffer.format(mLocalisation[34].p(),
	               mListSelect[0], mFolders.size(), mListSelect[1], mFiles.size());
	/* mBuffer.format("%ls %u / %u %ls %u / %u %ls",
		mLocalisation[34].p(),
		mListSelect[0], mFolders.size(), mLocalisation[34].p(),
		mListSelect[1], mFiles.size(), mLocalisation[34].p()); */
	mStatusbar->setText(0, mBuffer);
	return 1;
}

bool FileManager::addressEdited()
{
	// input address edited: mAddressEdit
	// input address index:  mListIndex

	_printf("address edited %d [%ls]\n", mListIndex, mAddressEdit.p());

	// C:\\ -> C:
	wchar_t* ac = mAddressEdit.p();
	uint len = mAddressEdit.length() - 1;
	while(ac[len] == 0x5C or ac[len] == 0x2F)
		ac[len--] = 0;
	mAddressEdit.update();

	// c: -> C:
	wchar_t cup = reinterpret_cast<wchar_t>(CharUpperW(reinterpret_cast<wchar_t*>(*ac)));
	*ac = cup;
	// *ac = stdupc(*ac);

	_printf("address edited %d [%ls]\n", mListIndex, mAddressEdit.p());

	// skip address edited
	mBuffer = mAddressEdit;
	mBufferw = mAddressCurrent;
	mBuffer.tolower();
	mBufferw.tolower();
	if(mBuffer == mBufferw and mListIndex == -1)
	{
		_printf("skip address edited \n");
		return 0;
	}

	byte openDir = 0;

	if(fileExist(mAddressEdit))
	{
		mArchiveName = mAddressEdit;
		mReturn = openArchive();
		if(mReturn == 1)
			openDir = 1;
		else if(mReturn == -1)
			return 0;
	}

	if(folderExist(mAddressEdit))
		openDir = 2;

	if(openDir)
	{
		if(mAddressEdit.length() > 2)
		{
			// conversion input buffer with case sensivity
			mBuffer = mAddressEdit;
			while(mBuffer.length() > 2)
			{
				int bs = mBuffer.findr(0x5C);
				fileName(mBuffer, mBufferw);
				mBufferw.update();
				uint len = mBufferw.length();
				if(mBufferw.compare(mBuffer, bs + 1, -1) != 0)
					mencpy(mAddressEdit.p() + bs + 1, mBufferw.p(), len * 2);
				mBuffer.resize(bs);
			}
			mBufferw.clear();
			_printf("mAddressEdit %ls\n", mAddressEdit.p());
		}

		// remove address item
		if(mAddressItemCount > mDriveCount and
		        (mAddressEdit.length() == 2 or mAddressEdit[0] != mAddressCurrent[0]))
		{
			_printf("remove address item \n");
			mgAddress->deleteItem(mAddressItemCurrent);
			mAddressItemCount = mDriveCount;
			// select address item
			if(mAddressEdit.length() == 2 and mListIndex == -1)
			{
				_printf("select address item \n");
				mAddressItemCurrent = mAddressCurrent[0] - 0x43; // - 'C'
				mgAddress->setCurrentItem(mAddressItemCurrent);
			}
		}

		// close archive
		if(mFileSystemType == 2 and openDir == 2)
		{
			closeArchive();
		}

		_printf("folder exist \n");
		mAddressCurrent = mAddressEdit;
		openFolder();
	}
	// open dir in archive
	else if(mFileSystemType == 2)
	{
		String arcName = mArchive.getName();
		mBuffer.format("%s\\", arcName.p());
		uint len = mBuffer.length();
		mBufferw = mAddressEdit;
		mBuffer.tolower();
		mBufferw.tolower();
		mBufferw == mBuffer;
		if(mAddressEdit.length() > len and mBuffer == mBufferw)
		{
			mBuffer.format("%s\\", mAddressEdit.p() + len);
			if(fileExistArchive(mBuffer, 1))
			{
				_printf("folder exist \n");
				mAddressArchive = mBuffer;
				mAddressEdit.format("%s\\%s", arcName.p(), mBuffer.p());
				mAddressEdit.resize(mAddressEdit.length() - 1);
				mAddressCurrent = mAddressEdit;
				openFolder();
				openDir = 1;
			}
		}
	}
	return openDir != 0;
}

bool FileManager::addressSelected()
{
	mListIndex = mgAddress->getCurrentItem();
	_printf("address selected %u\n", mListIndex);
	// remove address item
	if(mAddressItemCount > mDriveCount)
	{
		_printf("  remove address item %u\n", mAddressItemCurrent);
		mgAddress->deleteItem(mAddressItemCurrent);
		mAddressItemCount = mDriveCount;
		if(mListIndex > mAddressItemCurrent)
			mListIndex--;
	}
	char address[3] = {0x43 + mListIndex, 0x3A, 0};
	mAddressCurrent = address;
	openFolder();
	return 1;
}

bool FileManager::listSelected()
{
	mListIndex = -1;
	getListSelect();
	_printf("list edited %d type %u\n", mItemIndex, mItemType);
	if(mItemType == 1) // dir
	{
		mBuffer.format("%s\\%s", mAddressCurrent.p(), mFolders[mItemIndex].p());
		mAddressCurrent = mBuffer;
		mBuffer.clear();
		if(mFileSystemType == 2)
		{
			if(mArchiveLevel == 0)
				mAddressArchive.format("%s\\", mFolders[mItemIndex].p());
			else
			{
				mBuffer.format("%s%s\\", mAddressArchive.p(), mFolders[mItemIndex].p());
				mAddressArchive = mBuffer;
			}
		}
	}
	if(mItemType == 2) // file
	{
		mReturn = 0;
		if(mFileSystemType == 1)
		{
			mArchiveName.format("%s\\%s", mAddressCurrent.p(), mFiles[mItemIndex].p());
			mReturn = openArchive();
		}
		if(mReturn == 1)
		{
			mAddressCurrent.format("%s\\%s", mAddressCurrent.p(), mFiles[mItemIndex].p());
			mItemType = 1;
		}
		else if(!mReturn)
		{
			openFile();
		}
	}
	else if(mItemType == 3) // up
	{
		_printf(" move up address [%s] lv %u \n", mAddressCurrent.p(), mFileSystemLevel);
		// level 1 remove address item
		if(mFileSystemLevel == 1)
		{
			_printf("  remove address item \n");
			mgAddress->deleteItem(mAddressItemCurrent);
			mAddressItemCount = mDriveCount;
			_printf("  select address item \n");
			mAddressItemCurrent = mAddressCurrent[0] - 0x43; // - 'C'
			mgAddress->setCurrentItem(mAddressItemCurrent);
		}
		// remove folder from address
		int bs = mAddressCurrent.findr(0x5C);
		// copy removed folder for next select
		mAddressUp.assign(mAddressCurrent, bs + 1);
		mAddressCurrent.resize(bs);

		if(mFileSystemType == 2)
		{
			if(mArchiveLevel > 1)
			{
				bs = mAddressArchive.findr(0x5C, mAddressArchive.length() - 2);
				mAddressArchive.resize(bs + 1);
			}
			else if(mArchiveLevel == 1)
				mAddressArchive.clear();
			else // level 0, out from archive
			{
				closeArchive();
			}
		}
	}
	if(mItemType == 1 or mItemType == 3)
	{
		_printf(" open address [%s] \n", mAddressCurrent.p());
		openFolder();
	}
	return 1;
}

bool FileManager::renameFile()
{
	// rename
	if(mFileSystemType == 2)
		return renameArchive();

	mListIndex = -1;
	getListSelect();
	int nItem = mListIndex;

	// source -> destination
	mBuffer.format("%s\\%s", mAddressCurrent.p(), mItemString);
	mBufferw.format("%s\\%s", mAddressCurrent.p(), mAddressEdit.p()); // new name

	_printf("list edited %d %ls %ls\n", mItemType, mBuffer.p(), mBufferw.p());

	if(mItemType == 1 and folderExist(mBufferw))
	{
		_printf("list skip \n");
		return 0;
	}
	else if(mItemType == 2 and fileExist(mBufferw))
	{
		_printf("list skip \n");
		return 0;
	}

	if(!MoveFile(mBuffer.p(), mBufferw.p()))
	{
		_printf("file not renamed\n");
		return 0;
	}
	_printf("file renamed\n");

	updateFileList();
	mgFileList->selectItem(nItem);

	return 1;
}

bool FileManager::openFolder()
{
	_printf("open folder [%s] \n", mAddressCurrent.p());

	int image = mDriveCount;
	if(mFileSystemType == 2)
		image++;

	// insert new item (cur address, not C:) in address panel
	if(mAddressItemCount == mDriveCount and mAddressCurrent.length() > 2)
	{
		// insert index
		mAddressItemCurrent = mAddressCurrent[0] - 0x42; // - 'C' + 1
		mgAddress->setItem(mAddressCurrent, mAddressItemCurrent, image, 1, 0);
		mgAddress->setCurrentItem(mAddressItemCurrent);
		mAddressItemCount++;
	}
	else if(mAddressItemCount > mDriveCount) // update current item
	{
		mgAddress->setItem(mAddressCurrent, mAddressItemCurrent, image, 1, 1);
		mgAddress->setCurrentItem(mAddressItemCurrent);
	}

	updateFileList();
	mgFileList->setFocus();

	// search dir for select after push button up
	mListIndex = 0;
	if(mAddressUp.length())
	{
		_printf(" search dir up [%s] \n", mAddressUp.p());
		bool found = 0;
		forn(mFolders.size())
		{
			if(mAddressUp == mFolders[i])
			{
				_printf(" search completed [%s] \n", mFolders[i].p());
				mListIndex = i; // select item after open
				found = 1;
				break;
			}
		}
		if(!found)
			forn(mFiles.size())
		{
			if(mAddressUp == mFiles[i])
			{
				_printf(" search file completed [%s] \n", mFiles[i].p());
				mListIndex = mFolders.size() + i; // select item after open
				found = 1;
				break;
			}
		}
		if(mButtonUp)
			mListIndex++;
		mAddressUp.clear();
	}

	mgFileList->selectItem(mListIndex);
	updateStatusbar();
	return 1;
}

bool FileManager::openFile()
{
	// launch file from file system
	if(mFileSystemType == 1)
	{
		mBuffer.format("%s\\%s", mAddressCurrent.p(), mItemString);
		mReturn = (int) ShellExecute(0, L"open", mBuffer.p(), 0, mAddressCurrent.p(), SW_SHOW);
		if(mReturn <= 32)
		{
			mReturn = 0;
			_printf("# file not launched %d [%ls]\n", mReturn, mBuffer.p());
			// File %s not launched
			mBuffer.format(mLocalisation[114].p(), mItemString);
			messageBox(mBuffer, PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
		}
	}
	else
	{
		return openFileArchive();
	}
	return mReturn;
}

bool FileManager::createFolder()
{
	if(mFileSystemType == 2) return 0;

	StringWidget stringModal;
	// Create folder, folder
	mReturn = stringModal.create(mLocalisation[18], mLocalisation[105]);
	if(!mReturn) return 0;

	mReturn = folderExist(mBuffer);
	if(mReturn) return 1;

	mBuffer.format("%s\\%s", mAddressCurrent.p(), stringModal.mBuffer.p());
	tas::createFolder(mBuffer, 1);
	mReturn = folderExist(mBuffer);
	return mReturn;
}

bool FileManager::deleteFiles()
{
	if(mListSelect[2] == 0)
	{
		// Before deleting select items in file list
		messageBox(mLocalisation[110], mLocalisation[107], MB_OK | MB_ICONWARNING);
		return 0;
	}

	/*
		MB_ICONHAND         'x'  red    circle
		MB_ICONQUESTION     '?'  blue   circle
		MB_ICONEXCLAMATION  '!'  yellow triangle
		MB_ICONASTERISK     'i'  blue   circle
	*/

	byte shiftKey = mKeyboard.getKeyState(VK_SHIFT);
	int icon = 0;
	String* msg = 0;
	if(mFileSystemType == 1)
	{
		icon = shiftKey ? MB_ICONHAND : MB_ICONQUESTION;
		msg = shiftKey ? &mLocalisation[111] : &mLocalisation[112];
		// "Delete selected items irrevocably?" : "Delete selected items in recycle bin?"
	}
	else
	{
		icon = MB_ICONQUESTION;
		msg = &mLocalisation[113];
		// msg = "Delete selected items from archive?";
	}
	// Deleting
	mReturn = messageBox(*msg, mLocalisation[107], MB_OKCANCEL | icon);
	if(mReturn != 1)
		return 0;

	if(mFileSystemType == 2)
		return deleteArchive();

	// from file system
	mListIndex = -1;
	int nItem = -1; // first
	while(getListSelect())
	{
		if(mItemType == 3) continue;
		if(nItem == -1) nItem = mListIndex;
		mBuffer.format("%s\\%s", mAddressCurrent.p(), mItemString);
		removeFileShell(mBuffer, !shiftKey);
		_printf("remove %s %d %d\n", mBuffer.p(), mBuffer.length(), shiftKey);
	}

	updateFileList();
	int count = mgFileList->getItemCount(0);
	if(nItem >= count)
		nItem = count - 1;
	mgFileList->selectItem(nItem);

	return 1;
}

bool FileManager::information()
{
	if(mFileSystemType == 1)
	{
		mListIndex = -1;
		while(getListSelect())
		{
			if(mItemType == 3) continue;
			mBuffer.format("%s\\%s", mAddressCurrent.p(), mItemString);
			if(mItemType == 2)
			{
				mReturn = mArchive.open(mBuffer, 2);
				if(mReturn)
				{
					ArchiveInfo inform;
					inform.create();
					mArchive.close(1);
					continue;
				}
				else
					mArchive.close(1);
			}
			SHELLEXECUTEINFO info = {0};
			info.cbSize = sizeof(SHELLEXECUTEINFO);
			info.lpFile = mBuffer.p();
			info.nShow = SW_SHOW;
			info.fMask = SEE_MASK_INVOKEIDLIST;
			info.lpVerb = L"properties";
			ShellExecuteEx(&info);
		}
	}
	else
	{
		ArchiveInfo inform;
		inform.create();
	}
	return 1;
}

bool FileManager::launchManual()
{
	GetModuleFileName(0, mBuffer.p(), mBuffer.capacity());
	mBuffer.update();
	forn(2) mBuffer.trim(0x5C, 1);
	mBufferw = "\\doc\\html\\index.html";
	mBuffer += mBufferw;
	mReturn = (int) ShellExecute(0, L"open", mBuffer.p(), 0, 0, SW_SHOW);
	if(mReturn <= 32)
		_printf("# manual not launch %d\n", mReturn);
	return 1;
}

bool FileManager::launchLog()
{
	mReturn = (int) ShellExecute(0, L"open", getLogName().p(), 0, 0, SW_SHOW);
	if(mReturn <= 32)
		_printf("# log not launch %d\n", mReturn);
	return 1;
}

bool FileManager::updateAddress()
{
	if(mFileSystemType == 1 and TIMEMS - mTimeUpd > 2000)
	{
		bool update = 0;
		mTimeUpd = TIMEMS;
		// last modified time
		if(mFileSystemLevel != 0)
		{
			uint time[2] = {0};
			mReturn = fileTimeValue(mAddressCurrent, time, 2);
			if(mReturn and (mAddressTime[0] != time[0] or mAddressTime[1] != time[1]))
				update = 1;
		}
		// root hash file names
		else
		{
			mSearch.countFiles(mAddressCurrent);
			if(mSearch.hash != mRootHash)
				update = 1;
		}
		if(update)
		{
			mListIndex = -1;
			int nItem = 0; // first
			getListSelect();
			if(mListIndex != -1) nItem = mListIndex;
			updateFileList();
			int count = mgFileList->getItemCount(0);
			if(nItem >= count)
				nItem = count - 1;
			mgFileList->selectItem(nItem);
		}
	}
	return 1;
}

bool FileManager::getFileType(String& file, String& type)
{
#define CMP(s,n) (type.length() == n and type.compare(L##s, n) == 0)
	String ext;
	int dot = file.findr(0x2E);
	if(dot != -1)
	{
		// .ext
		type.assign(file, dot + 1);
		type.tolower();

		if(CMP("exe", 3))
			type = mLocalisation[122]; // Executable
		else if(CMP("dll", 3))
			type = mLocalisation[123]; // Dynamic Library
		else if(CMP("bat", 3) or CMP("cmd", 3))
			type = mLocalisation[124]; // Batch
		else if(CMP("arv", 3) or CMP("zip", 3) or CMP("7z", 2) or CMP("rar", 3))
			type = mLocalisation[125]; // Archive
		else if(CMP("sys", 3))
			type = mLocalisation[126]; // System
		else if(CMP("cpp", 3) or CMP("c", 1))
			type = mLocalisation[127]; // Source file C / C++
		else if(CMP("hpp", 3) or CMP("h", 1))
			type = mLocalisation[128]; // Header file C / C++
		else if(CMP("txt", 3))
			type = mLocalisation[129]; // Text
		else if(CMP("fb2", 3))
			type = mLocalisation[130]; // Book
		else if(CMP("pdf", 3))
			type = mLocalisation[131]; // Document
		else if(CMP("html", 4) or CMP("htm", 3))
			type = mLocalisation[132]; // Web document
	}
	else
		type = mLocalisation[0]; // File
#undef CMP
	return 1;
}

bool FileManager::readRegistry(byte state)
{
	if(state and mSystemSegregation)
		return 1;

	Registry registry;
	int regRet = registry.open(HKEY_CURRENT_USER, "Software\\Archivarius\\Options", !state);
	_printf("Registry open state %u \n", regRet);
	if(!regRet)
	{
		_printf("Registry not exist \n");
		if(!state)
			return 0;
	}

	_printu(mArchiverSettings.fullRowSelect);

	// 21 items
	char* params[] =
	{
		"Localisation", "Address",

		"WindowPosX", "WindowPosY",
		"WindowWidth", "WindowHeight",

		"WindowMaximized", "WindowAlign", "FullRowSelect",
		"PasswordCryptFile", "PasswordShow", "FilePaths",
		"SortType", "SortMode",

		"Level", "Method", "Dictionary",
		"MatchMin", "MatchMax", "Threads", "Bigraph"
	};

	// read
	if(!state)
	{
		_printf("Registry reading \n");
		forn(21)
		{
			uint len = BUFFER_SIZE;
			regRet = registry.getValue(params[i], reinterpret_cast<byte*>(mBuffer.p()), &len);
			mBuffer.update();
			if(!regRet)
			{
				_printf("Registry not readed %u\n", i);
				continue;
			}
			if(i == 0)
				mLocalisationId = mBuffer;
			else if(i == 1)
				mAddressCurrent = mBuffer;
			else if(i < 4)
				mWindowPos[i-2] = *((uint*)mBuffer.p());
			else if(i < 6)
				mWindowSize[i-4] = *((uint*)mBuffer.p());
			else if(i < 14) // crypt, show, file path
				mArchiverSettings[i+1] = *((uint*)mBuffer.p());
			else
				mArchiverSettings[i-14] = *((uint*)mBuffer.p());
			if(i == 13)
			{
				registry.close();
				regRet = registry.open(HKEY_CURRENT_USER, "Software\\Archivarius\\Options\\Compression", !state);
				if(!regRet)
					return 0;
			}
		}
		registry.close();
		mSortType = mArchiverSettings.sortType;
		mSortMode = mArchiverSettings.sortMode;
	}
	else // write
	{
		if(!regRet)
			regRet = registry.create(HKEY_CURRENT_USER, "Software\\Archivarius\\Options");
		if(!regRet)
			return 0;

		mArchiverSettings.sortType = mSortType;
		mArchiverSettings.sortMode = mSortMode;

		if(mFileSystemType == 2)
		{
			mAddressCurrent = mArchive.getName();
			mAddressCurrent.trim(0x5C, 1);
		}
		if(mArchiverSettings.level == 0) // encode level
		{
			forn(6)
			mArchiverSettings[i+1] = -1;
		}
		uint value = 0;
		mWindowPos = mWindow->getPosition();
		mWindowSize = mWindow->getSize();
		mArchiverSettings.windowMaximize = mWindow->isMaximized();

		forn(21)
		{
			if(i == 0)
				regRet = registry.setValue(params[i], REG_SZ, reinterpret_cast<byte*>(mLocalisationId.p()), mLocalisationId.length() * 2);
			else if(i == 1)
				regRet = registry.setValue(params[i], REG_SZ, reinterpret_cast<byte*>(mAddressCurrent.p()), mAddressCurrent.length() * 2);
			else if(i < 4)
			{
				value = mWindowPos[i-2];
				regRet = registry.setValue(params[i], REG_DWORD, (byte*)&value, 4);
			}
			else if(i < 6)
			{
				value = mWindowSize[i-4];
				regRet = registry.setValue(params[i], REG_DWORD, (byte*)&value, 4);
			}
			else if(i < 14)
			{
				value = mArchiverSettings[i+1];
				regRet = registry.setValue(params[i], REG_DWORD, (byte*)&value, 4);
			}
			else
			{
				value = mArchiverSettings[i-14];
				regRet = registry.setValue(params[i], REG_DWORD, (byte*)&value, 4);
			}
			if(!regRet)
			{
				_printf("Registry not writed %u\n", i);
				return 0;
			}
			if(i == 13)
			{
				// version
				mBuffer.format("%u.%u.%u", TAA_VERSION_MAJOR, TAA_VERSION_MINOR, TAA_VERSION_PATCH);
				regRet = registry.setValue("Version", REG_SZ, reinterpret_cast<byte*>(mBuffer.p()), mBuffer.length() * 2);
				registry.close();
				regRet = registry.open(HKEY_CURRENT_USER, "Software\\Archivarius\\Options\\Compression", !state);
				if(!regRet)
					regRet = registry.create(HKEY_CURRENT_USER, "Software\\Archivarius\\Options\\Compression");
				if(!regRet)
					return 0;
			}
		}
		_printf("Registry writed \n");
		registry.close();
	}

	mBuffer.clear();
	return 1;
}

bool FileManager::readLocalisation()
{
	GetModuleFileName(0, mBuffer.p(), mBuffer.capacity());
	mBuffer.update();
	mBuffer.trim(0x5C, 1);
	if(mLocalisationId == "en")
		mBuffer.append(L"\\localisation\\English.txt");
	else if(mLocalisationId == "ru")
		mBuffer.append(L"\\localisation\\Russian.txt");
	TextStream text(mBuffer, 1, "", 512);
	assert(text.state());
	stdprintf("localisation\n\n");
	for(int i = 0; text.readLine();)
	{
		mBuffer = text.getLine();
		if(mBuffer.length() == 0 or mBuffer[0] == ';') continue;
		mLocalisation.push_back(mBuffer);
		stdprintf("[%02u] %ls\n", i++, mBuffer.p());
	}
	stdprintf("\n\n");
	return 1;
}

bool FileManager::createContextMenu()
{
	Vector2i mousePos(WT_LOWORD(mListIndex), WT_HIWORD(mListIndex));

	// menu file list
	if(mousePos.x == 0xFFFF)
	{
		POINT pt;
		GetCursorPos(&pt);
		mousePos = Vector2i(pt.x, pt.y);
	}
	else
	{
		uint min = mFileListPos.y + 20;
		uint max = mFileListPos.y + mFileListSize.y - 20;
		POINT pt = {mousePos.x, mousePos.y};
		ScreenToClient(mWindow->getHandle(), &pt);
		if(pt.y < min or pt.y > max)
			return 0;
	}

	Menu cxMenu;
	cxMenu.create(1);
	mBuffer.format("%s\tF8", mLocalisation[10].p());
	cxMenu.appendItem(ID_INFO,      mBuffer);
	mBuffer.format("%s\tF2", mLocalisation[12].p());
	cxMenu.appendItem(ID_MC_RENAME, mBuffer);
	mBuffer.format("%s\tDel", mLocalisation[11].p());
	cxMenu.appendItem(ID_DELETE,    mBuffer);
	mBuffer.format("%s\tCtrl+C", mLocalisation[16].p());
	cxMenu.appendItem(ID_MC_COPY,   mBuffer);
	mBuffer.format("%s\tCtrl+V", mLocalisation[17].p());
	cxMenu.appendItem(ID_MC_PASTE,  mBuffer);
	cxMenu.appendSeparator();
	mBuffer.format("%s\tCtrl+O", mLocalisation[1].p());
	cxMenu.appendItem(ID_MF_OPEN,   mBuffer);
	mBuffer.format("%s\tCtrl+W", mLocalisation[2].p());
	cxMenu.appendItem(ID_MF_CLOSE,  mBuffer);
	if(mFileSystemType == 1)
	{
		mBuffer.format("%s\tCtrl+F7", mLocalisation[18].p());
		cxMenu.appendItem(ID_MC_NEWDIR, mBuffer);
	}
	cxMenu.appendSeparator();
	mBuffer.format("%s\tCtrl+F3", mLocalisation[13].p());
	cxMenu.appendItem(ID_MC_UPDATE, mBuffer);
	cxMenu.trackMenu(mWindow, mousePos, TPM_LEFTALIGN | TPM_RIGHTBUTTON);
	cxMenu.destroy();

	return 1;
}

bool FileManager::copyClipboard()
{
	if(!mListSelect[2]) return 0;
	if(mFileSystemType == 2)
		return copyClipboardArchive();
	copyClipboardImpl(0);
	return 1;
}

bool FileManager::copyClipboardImpl(byte type)
{
	if(!OpenClipboard(0))
		return 0;
	mListIndex = -1;
	wchar_t* address = type ? mDirectoryUnpack.p() : mAddressCurrent.p();
	mBuffer.clear();
	while(getListSelect())
	{
		if(mItemType == 3) continue;
		mBufferw.format("%s\\%s", address, mItemString);
		uint capacity = mBuffer.capacity();
		uint newCapacity = mBufferw.length() + mBufferw.length() + 32;
		if(newCapacity > capacity)
		{
			while(newCapacity > capacity)
				capacity += BUFFER_SIZE;
			mBuffer.reserve(capacity);
		}
		mBuffer.append(mBufferw.p(), mBufferw.length() + 1);
	}
	// last nil
	mBuffer.resize(mBuffer.length() + 1);
	HGLOBAL hGlobal = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, mBuffer.length() * 2 + sizeof(DROPFILES));
	if(!hGlobal) return 0;
	void* clipMem = GlobalLock(hGlobal);
	DROPFILES* dropFiles = static_cast<DROPFILES*>(clipMem);
	dropFiles->pFiles = sizeof(DROPFILES);
	dropFiles->fWide = 1; // wide char
	char* pWrite = static_cast<char*>(clipMem) + sizeof(DROPFILES);
	mencpy(pWrite, mBuffer.p(), mBuffer.length() * 2);
	EmptyClipboard();
	SetClipboardData(CF_HDROP, hGlobal);
	CloseClipboard();
	GlobalUnlock(hGlobal);
	mBuffer.clear();
	_printf("files copied to clipboard\n");
	return 1;
}

bool FileManager::pasteClipboard()
{
	if(!OpenClipboard(0))
		return 0;

	HGLOBAL hGlobal = GetClipboardData(CF_HDROP);
	if(hGlobal != 0)
	{
		HDROP hDrop = (HDROP)GlobalLock(hGlobal);
		mDragDrop.setHandle((void*)hDrop);
		GlobalUnlock(hGlobal);
		setCommand(ID_APPEND);
		mAppendType = 1;
	}

	CloseClipboard();
	return 1;
}

int FileManager::callback(Widget* widget, uint message, uint_t param)
{
	if(mStop) return 1;
	switch(message)
	{
	case EVM_ACTIVATE:
	{
		setCommand(FMC_ACTIVATE);
		break;
	}
	case EVM_CLICKED:
	{
		onButtons(param);
		break;
	}
	case EVM_SIZE:
	{
		setCommand(FMC_SIZE);
		break;
	}
	case EVM_LIST_UPDATE:
	{
		setCommand(FMC_LIST_UPDATE);
		break;
	}
	case EVM_LIST_DBLCLK:
	{
		setCommand(FMC_LIST_SELECT);
		break;
	}
	case EVM_LIST_COLCLK:
	{
		if(mSortType == param)
			mSortMode = !mSortMode;
		else
			mgFileList->showArrow(mSortType, 0);
		mSortType = param;
		mSortList = 1;
		setCommand(FMC_LIST_COLUMN);
		break;
	}
	case EVM_LIST_KEYDOWN:
	{
		break;
		uint* params = (uint*) param;
		if(params[1] == VK_RETURN)
			setCommand(FMC_LIST_SELECT);
		else if(params[1] == VK_BACK)
			setCommand(FMC_LIST_BACK);
		break;
	}
	case EVM_CB_SELCHANGE:
	{
		setCommand(FMC_ADDRESS_SELECT);
		break;
	}
	case EVM_CB_ENDEDIT:
	{
		uint_t* params = (uint_t*) param;
		mListIndex = params[1];
		mAddressEdit = (wchar_t*) params[2];
		_printf("mAddressEdit %ls\n", mAddressEdit.p());
		setCommand(FMC_ADDRESS_EDITED);
		break;
	}
	case EVM_CB_KILLFOCUS:
	{
		setCommand(FMC_ADDRESS_KILLFOCUS);
		mKeyboard.setKeyState(VK_RETURN, 1);
		break;
	}
	case EVM_LIST_BEGEDIT:
	{
		uint_t* params = (uint_t*) param;
		if(mButtonUp and !params[1])
			return 0;
		mListEdit = 1;
		break;
	}
	case EVM_LIST_ENDEDIT:
	{
		mListEdit = 0;
		uint_t* params = (uint_t*) param;
		wchar_t* edited = (wchar_t*) params[2];
		if(stwlen(edited))
		{
			setCommand(FMC_LIST_EDIT);
			mKeyboard.setKeyState(VK_RETURN, 1);
			mItemIndex = params[1];
			mAddressEdit = edited;
		}
		break;
	}
	case EVM_DROP:
	{
		mDragDrop.setHandle((void*)param);
		setCommand(ID_APPEND);
		mAppendType = 1;
		break;
	}
	case EVM_CONTEXTMENU:
	{
		mListIndex = param;
		createContextMenu();
		break;
	}
	}
	return 1;
}

void FileManager::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
	updateAddress();
}

void FileManager::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case ID_APPEND:
			appendFiles();
			break;
		case ID_EXTRACT:
			extractFiles();
			break;
		case ID_TEST:
			testArchive();
			break;
		case ID_INFO:
			information();
			break;
		case ID_DELETE:
			deleteFiles();
			break;
		case ID_MF_OPEN:
			openArchiveDialog();
			break;
		case ID_MF_CLOSE:
			closeArchiveMenu();
			break;
		case ID_MF_OPEN_LOG:
			launchLog();
			break;
		case ID_MF_OPTIONS:
		{
			ArchiverOptions options;
			options.create();
			break;
		}
		case FMC_EXIT:
			mWindow->close();
			break;
		case FMC_SIZE:
			onSize();
			break;
		case FMC_LIST_UPDATE:
			updateStatusbar();
			break;
		case FMC_ADDRESS_EDITED:
			addressEdited();
			break;
		case FMC_ADDRESS_SELECT:
			addressSelected();
			break;
		case FMC_DEACTIVATE:
			mWindow->setActivate(0);
			break;
		case FMC_ACTIVATE:
		case FMC_ADDRESS_KILLFOCUS:
			mgFileList->setFocus();
			break;
		case FMC_LIST_SELECT:
			listSelected();
			break;
		case FMC_LIST_COLUMN:
			updateFileList();
			break;
		case FMC_LIST_EDIT:
			renameFile();
			break;
		case FMC_LIST_BACK:
			if(mFileSystemLevel)
			{
				mgFileList->selectItem(0);
				listSelected();
			}
			break;
		case ID_MC_RENAME:
			if(mListSelect[2] == 1)
			{
				mListIndex = -1;
				getListSelect();
				mgFileList->editLabel(mListIndex);
			}
			break;
		case ID_MC_UPDATE:
		{
			mListIndex = -1;
			getListSelect();
			int nItem = mListIndex;
			updateFileList();
			if(nItem == -1)
				nItem = 0;
			mgFileList->selectItem(nItem);
			break;
		}
		case ID_MC_SELADR:
			mgFileList->setFocused(0);
			if(!mgAddress->isFocused())
				mgAddress->setFocus();
			break;
		case ID_MC_SELALL:
			mgFileList->selectAll();
			break;
		case ID_MC_NEWDIR:
			createFolder();
			break;
		case ID_MC_COPY:
			copyClipboard();
			break;
		case ID_MC_PASTE:
			pasteClipboard();
			break;
		case ID_MV_NAME:
			callback(mWindow, EVM_LIST_COLCLK, 0);
			break;
		case ID_MV_SIZE:
			callback(mWindow, EVM_LIST_COLCLK, 1);
			break;
		case ID_MV_TYPE:
			callback(mWindow, EVM_LIST_COLCLK, 2);
			break;
		case ID_MV_TIME:
			callback(mWindow, EVM_LIST_COLCLK, 3);
			break;
		case ID_MH_MANUAL:
		{
			launchManual();
			break;
		}
		case ID_MH_ABOUT:
		{
			mKeyboard.setKeyState(VK_RETURN, 1);
			ArchiverAbout aboutFrame;
			aboutFrame.create();
			break;
		}
		}
	}
	mCommands.resize(0);
}

void FileManager::keyboard()
{
	if(mKeyboard.getKeyPressed(VK_F12))
	{
		setCommand(FMC_EXIT);
	}
	else if(mKeyboard.getKeyPressed(VK_F4))
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MV_TYPE);
		else
			setCommand(ID_MC_SELADR);
	}

	// address focused, exit
	if(mgAddress->isFocused())
		return;

	if(mKeyboard.getKeyPressed(VK_RETURN))
	{
		listSelected();
	}
	else if(mKeyboard.getKeyPressed(VK_BACK))
	{
		if(mFileSystemLevel and !mListEdit)
		{
			mgFileList->selectItem(0);
			listSelected();
		}
	}
	else if(mKeyboard.getKeyPressed(VK_F1))
	{
		setCommand(ID_MH_MANUAL);
	}
	else if(mKeyboard.getKeyPressed(VK_F2))
	{
		setCommand(ID_MC_RENAME);
	}
	else if(mKeyboard.getKeyPressed(VK_F3))
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MV_NAME);
		else
			setCommand(ID_MC_UPDATE);
	}
	else if(mKeyboard.getKeyPressed(VK_F5))
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MV_TIME);
		else
			setCommand(ID_APPEND);
	}
	else if(mKeyboard.getKeyPressed(VK_F6))
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MV_SIZE);
		else
			setCommand(ID_EXTRACT);
	}
	else if(mKeyboard.getKeyPressed(VK_F7))
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MC_NEWDIR);
		else
			setCommand(ID_TEST);
	}
	else if(mKeyboard.getKeyPressed(VK_F8))
	{
		setCommand(ID_INFO);
	}
	else if(mKeyboard.getKeyPressed(VK_F11))
	{
		setCommand(ID_MF_OPTIONS);
	}
	else if(mKeyboard.getKeyPressed(VK_DELETE))
	{
		setCommand(ID_DELETE);
	}
	else if(mKeyboard.getKeyPressed(0x41)) // A
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MC_SELALL);
	}
	else if(mKeyboard.getKeyPressed(0x4F)) // O
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MF_OPEN);
	}
	else if(mKeyboard.getKeyPressed(0x57)) // W
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MF_CLOSE);
	}
	else if(mKeyboard.getKeyPressed(0x4C)) // L
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MF_OPEN_LOG);
	}
	else if(mKeyboard.getKeyPressed(0x43)) // C
	{
		if(mKeyboard.getKeyState(VK_CONTROL))
		{
			if(mListSelect[2])
				setCommand(ID_MC_COPY);
		}
	}
	else if(mKeyboard.getKeyPressed(0x56)) // V
	{
		// paste clipboard
		if(mKeyboard.getKeyState(VK_CONTROL))
			setCommand(ID_MC_PASTE);
	}
}

void FileManager::onButtons(uint id)
{
	_printf("button clicked %d\n", id);
	switch(id)
	{
	case ID_APPEND:
	{
		setCommand(ID_APPEND);
		break;
	}
	case ID_EXTRACT:
	{
		setCommand(ID_EXTRACT);
		break;
	}
	case ID_TEST:
	{
		setCommand(ID_TEST);
		break;
	}
	case ID_INFO:
	{
		setCommand(ID_INFO);
		break;
	}
	case ID_DELETE:
	{
		setCommand(ID_DELETE);
		break;
	}
	case ID_MF_OPEN:
	{
		setCommand(ID_MF_OPEN);
		break;
	}
	case ID_MF_CLOSE:
	{
		setCommand(ID_MF_CLOSE);
		break;
	}
	case ID_MF_OPEN_LOG:
	{
		setCommand(ID_MF_OPEN_LOG);
		break;
	}
	case ID_MF_OPTIONS:
	{
		setCommand(ID_MF_OPTIONS);
		break;
	}
	case ID_MF_EXIT:
	{
		mStop = 1;
		setCommand(FMC_EXIT);
		break;
	}
	case ID_MC_RENAME:
	{
		setCommand(ID_MC_RENAME);
		break;
	}
	case ID_MC_UPDATE:
	{
		setCommand(ID_MC_UPDATE);
		break;
	}
	case ID_MC_SELADR:
	{
		setCommand(ID_MC_SELADR);
		break;
	}
	case ID_MC_SELALL:
	{
		setCommand(ID_MC_SELALL);
		break;
	}
	case ID_MC_NEWDIR:
	{
		setCommand(ID_MC_NEWDIR);
		break;
	}
	case ID_MC_COPY:
	{
		setCommand(ID_MC_COPY);
		break;
	}
	case ID_MC_PASTE:
	{
		setCommand(ID_MC_PASTE);
		break;
	}
	case ID_MV_NAME:
	{
		setCommand(ID_MV_NAME);
		break;
	}
	case ID_MV_TYPE:
	{
		setCommand(ID_MV_TYPE);
		break;
	}
	case ID_MV_TIME:
	{
		setCommand(ID_MV_TIME);
		break;
	}
	case ID_MV_SIZE:
	{
		setCommand(ID_MV_SIZE);
		break;
	}
	case ID_MH_MANUAL:
	{
		setCommand(ID_MH_MANUAL);
		break;
	}
	case ID_MH_ABOUT:
	{
		setCommand(ID_MH_ABOUT);
		break;
	}
	}
}

void FileManager::onSize()
{
	mWindowSize = mWindow->getSize();
	mAddressSize = Vector2i(mWindowSize.x, mgAddress->getSize().y);
	mFileListSize = Vector2i(mWindowSize.x, mWindowSize.y - mFileListPos.y - mStatusbar->getSize().y - 2);
	mFileListPos.y = mAddressPos.y + mAddressSize.y + 3;
	mgAddress->move(mAddressPos, mAddressSize);
	mgFileList->move(mFileListPos, mFileListSize);
}

void FileManager::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

int FileManager::messageBox(const String& message, const String& caption, uint type)
{
	mReturn = MessageBox(mWindow->getHandle(), message.p(), caption.p(), type);
	mKeyboard.setKeyState(VK_RETURN, 1);
	return mReturn;
}

FileManager* FileManager::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return FileManager::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	FileManager::ptr()->idle();
}

}