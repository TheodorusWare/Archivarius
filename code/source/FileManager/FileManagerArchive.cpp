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
#include <FileManager/ProgressWidget.h>
#include <FileManager/PasswordWidget.h>
#include <Common/StringLib.h>
#include <Common/String.h>
#include <Common/FileAccess.h>
#include <Common/Registry.h>
#include <Compress/Archive.h>
#include <time.h>

namespace tas
{

int EventCallbackImpl(wint* data); // ProgressWidget.cpp
String& getLogName();

bool FileManager::appendFiles()
{
	// not supported inside archive
	if(mFileSystemType == 2 and !mAppendType)
	{
		// Operation not supported inside archive
		messageBox(mLocalisation[115], mLocalisation[89], MB_OK | MB_ICONWARNING);
		return 0;
	}

	if(!mListSelect[2] and !mAppendType)
	{
		// Before archiving select items in file list
		messageBox(mLocalisation[108], mLocalisation[89], MB_OK | MB_ICONWARNING);
		return 0;
	}

	mArchiveFiles.resize(0);
	int nItem = -1; // first

	// list
	if(!mAppendType)
	{
		mListIndex = -1;
		while(getListSelect())
		{
			if(mItemType == 1) // dir
			{
				mBuffer.format("%s\\%s", mAddressCurrent.p(), mFolders[mItemIndex].p());
				findFiles(mBuffer, 1, mArchiveFiles);
			}
			else if(mItemType == 2) // file
			{
				mBuffer.format("%s\\%s", mAddressCurrent.p(), mFiles[mItemIndex].p());
				mArchiveFiles.push_back(mBuffer);
			}
			if(nItem == -1) nItem = mListIndex;
		}
	}
	else // drop
	{
		mListIndex = -1;
		getListSelect();
		if(mListIndex != -1)
			nItem = mListIndex;
		else
			nItem = 0;
		uint count = mDragDrop.getCount();
		forn(count)
		{
			mDragDrop.getItem(i, mBuffer);
			mBuffer.update();
			if(folderExist(mBuffer))
				findFiles(mBuffer, 1, mArchiveFiles);
			else if(fileExist(mBuffer))
				mArchiveFiles.push_back(mBuffer);
			if(!i)
			{
				// skip file len
				mItemIndex = mBuffer.findr(0x5C) + 1;
			}
		}
	}

	if(!mArchiveFiles.size())
	{
		// Before archiving select items in file list
		messageBox(mLocalisation[108], mLocalisation[89], MB_OK | MB_ICONWARNING);
		return 0;
	}

	// archive name
	if(!mAppendType)
	{
		if(mListSelect[2] == 1) // total
		{
			mListIndex = -1;
			getListSelect();
			if(mItemType == 1) // dir
				mArchiveName = mFolders[mItemIndex];
			else if(mItemType == 2) // file
			{
				mArchiveName = mFiles[mItemIndex];
				mArchiveName.trim(0x2E, 1);
			}
		}
		else // multi select
		{
			// extract archive name
			mArchiveName = mAddressCurrent.substrx(0x5C, 1, 1);
			if(!mFileSystemLevel)
				mArchiveName.resize(1);
		}
	}
	else // drop
	{
		if(mFileSystemType == 1)
		{
			uint count = mDragDrop.getCount();
			mDragDrop.getItem(0, mBuffer);
			mBuffer.update();

			// archive name
			if(count == 1)
			{
				mArchiveName = mBuffer.substr(mItemIndex);
				mArchiveName.trim(0x2E, 1);
			}
			else
			{
				// delimeter nil
				mBuffer.resize(mItemIndex-1);
				int bs = mBuffer.findr(0x5C);
				if(bs != -1)
					mArchiveName.assign(mBuffer, bs + 1);
				else
				{
					mArchiveName.assign(mBuffer, 0, 1);
				}
			}
		}
		else
		{
			String arcName = mArchive.getName();
			mArchiveDirectory = arcName.substrx(0x5C, 1, 0);
			mArchiveName = arcName.substrx(0x5C, 1, 1);
			mArchiveName.trim(0x2E, 1);
		}
	}

	// .ext
	mArchiveName.append(ARCHIVE_EXT);
	mPassword.clear();

	if(mFileSystemType == 1)
		mArchiveDirectory = mAddressCurrent;

	// Open encode settings modal window
	EncodeSettings encodeSettings;
	int mReturn = encodeSettings.create(mArchiveDirectory, mArchiveName);

	if(!mReturn)
	{
		mAppendType = 0;
		return 0;
	}

	mReturn = createArchive();
	updateFileList();
	mgFileList->setFocus();
	mgFileList->selectItem(nItem);
	mAppendType = 0;

	return mReturn;
}

bool FileManager::extractFiles()
{
	// extract files from opened archive

	if(mListSelect[2] == 0)
	{
		// Before unarchiving select items in file list
		messageBox(mLocalisation[109], mLocalisation[90], MB_OK | MB_ICONWARNING);
		return 0;
	}

	// extract from file system
	if(mFileSystemType == 1)
		return extractFilesSys();

	mBuffer = mArchive.getName();
	mBuffer.trim(0x2E, 1);
	DecodeSettings decodeSettings;
	int mReturn = decodeSettings.create(mBuffer);

	if(!mReturn)
		return 0;

	mArchiveDirectory = DS->mDirectory;

	mReturn = prepareExtract();
	if(!mReturn) return 0;

	mUnpackType = 5;
	mReturn = extractArchive();

	if(!mReturn)
	{
		_printf("unpack error [%ls] %u\n", mArchive.getErrorStr().p(), mArchive.getError());
		mBufferw.assign(mArchive.getName()).substrxs(0x5C, 1, 1);
		// Unarchiving %s completed with errors!
		mBuffer.format(mLocalisation[116].p(), mBufferw.p());
		messageBox(mBuffer, mLocalisation[90], MB_OK | MB_ICONWARNING);
	}

	return mReturn;
}

bool FileManager::extractFilesSys()
{
	// extract archives from file system

	mReturn = 0;
	int dialog = 0; // unpack

	// count archives
	/*
		total
		unpacked
		not unpacked
	*/
	uint nCount[3] = {0};

	mListIndex = -1;
	int nItem = -1; // first
	while(getListSelect())
	{
		if(mItemType == 3) continue;
		if(nItem == -1) nItem = mListIndex;
		if(mItemType == 2) // file
		{
			bool needPassword = 0;
			mArchiveName.format("%s\\%s", mAddressCurrent.p(), mItemString);
			mArchive.close(1);
			mReturn = mArchive.open(mArchiveName, 1);
			if(!mReturn)
			{
				if(mArchive.getError() != 4)
					continue;
				else
					needPassword = 1;
			}
			if(!dialog)
			{
				dialog = 1;
				mBuffer = mArchiveName;
				mBuffer.trim(0x2E, 1);
				DecodeSettings decodeSettings;
				mReturn = decodeSettings.create(mBuffer);
				if(!mReturn)
				{
					mArchive.close(1);
					return 0;
				}
				mArchiveDirectory = DS->mDirectory;
			}
			nCount[0]++; // total
			if(needPassword)
			{
				if(!queryPassword(0))
					continue;
			}
			mUnpackType = 1;
			mReturn = extractArchive();
			if(mReturn == -1) // cancel
				return 0;
			if(mReturn)
				nCount[1]++; // unpacked
			else if(!mReturn)
			{
				_printf("error [%ls] %u\n", mArchive.getErrorStr().p(), mArchive.getError());
				nCount[2]++; // errors
				// Unarchiving %s completed with errors!
				mBuffer.format(mLocalisation[116].p(), mItemString);
				messageBox(mBuffer, mLocalisation[90], MB_OK | MB_ICONWARNING);
			}
			mArchive.close(1);
		}
	}

	// archives not found
	if(nCount[0] == 0)
	{
		mReturn = 0;
		// Archives not found!
		messageBox(mLocalisation[117], mLocalisation[90], MB_OK | MB_ICONWARNING);
	}

	updateFileList();
	mgFileList->selectItem(nItem);

	mUnpackType = 0;
	return mReturn;
}

bool FileManager::createArchive()
{
	uint skipLen = 0;

	if(mFileSystemType == 1)
	{
		mArchiveName.format("%s\\%s", ES->mArchiveDirectory.p(), ES->mArchiveNames.p());
	}
	else // inside archive, drop files
	{
		backupArchive(0);
	}

	EncodeParams* ep = &ES->mEncodeParams;

	if(!mAppendType)
		skipLen = mArchiveDirectory.length() + 1;
	else
		skipLen = mItemIndex;

	int mReturn = 0;
	if(mFileSystemType == 1 and fileExist(mArchiveName))
	{
		mReturn = mArchive.open(mArchiveName, 1);
		if(mReturn == 1)
		{
			if(ep->crypt and mArchive.getValue(5))
				mArchive.saveCrypt();
		}
		else
		{
			// password incomplete, not rewrite file
			if(mArchive.getError() == 4)
			{
				// backup current password
				bool pass = 0;
				mBufferw.clear();
				if(ep->crypt and mPassword.length())
				{
					mBufferw = mPassword;
					pass = 1;
				}
				mReturn = queryPassword(0);
				if(!mReturn) return 0;
				if(ep->crypt)
				{
					mArchive.saveCrypt();
					mPassword.clear();
					if(pass)
						mPassword = mBufferw;
					else
						mArchive.resetCrypt();
				}
			}
			else
				mArchive.close(1);
		}
	}

	if(mFileSystemType == 2)
	{
		if(!mArchive.checkCrypt())
		{
			// backup current password
			bool pass = 0;
			mBufferw.clear();
			if(ep->crypt and mPassword.length())
			{
				mBufferw = mPassword;
				pass = 1;
			}
			mArchiveName = mArchive.getName();
			mReturn = queryPassword(1);
			if(!mReturn)
				return 0;
			if(ep->crypt)
			{
				mArchive.saveCrypt();
				mPassword.clear();
				if(pass)
					mPassword = mBufferw;
				else
					mArchive.resetCrypt();
			}
		}
		else if(ep->crypt and mArchive.getValue(5))
		{
			_linef;
			// crypt state
			// cur pass != def pass ?
			mArchive.saveCrypt();
			if(!mPassword.length())
			{
				_linef;
				mArchive.resetCrypt();
			}
		}
	}

	if(!mArchive.createLog(getLogName()))
	{
		_linef;
		return 0;
	}

	if(mFileSystemType == 1 and !mReturn)
	{
		mReturn = mArchive.create(mArchiveName);
		if(!mReturn)
		{
			_linef;
			return 0;
		}
	}

	if(ep->level)
	{
		mArchive.setValue(ep->method, 0);
		mArchive.setValue(ep->level, 1);
		mArchive.setValue(ep->dictSize, 3);
		mArchive.setValue(ep->matchMax, 4);
		mArchive.setValue(ep->matchMin, 9);
		mArchive.setValue(ep->bigraph, 16);
		if(ep->method == 1) // lzre
		{
			mArchive.setValue(ep->threads, 2);
			if(ep->matchCycles != 0xFFFFFFFF)
				mArchive.setValue(ep->matchCycles, 5);
			if(ep->suffix != 0xFFFFFFFF)
				mArchive.setValue(ep->suffix, 15);
			if(ep->rolz != 0xFFFFFFFF)
				mArchive.setValue(ep->rolz, 20);
			if(ep->cmix != 0xFFFFFFFF)
				mArchive.setValue(ep->cmix, 21);
		}
		else if(ep->method == 2) // cm
		{
		}
	}

	// password
	if(mPassword.length())
		mArchive.setValue((uint_t) mPassword.p(), 6);

	// crypt
	mArchive.setValue((uint_t) ep->crypt, 19);

	// set callback event
	mArchive.setValue((uint_t) EventCallbackImpl, 8);

	// skip file name
	mArchive.setValue((uint_t) skipLen, 17);

	// path in mArchive
	if(mFileSystemType == 2 and mArchiveLevel)
	{
		mBuffer.assign(mAddressArchive, 0, mAddressArchive.length() - 1);
		mArchive.setValue((uint_t) mBuffer.p(), 18);
	}

	// progress bar window
	ProgressWidget progress;
	progress.create(0);

	// append
	mReturn = mArchive.append(mArchiveFiles);

	if(!mReturn)
		_printf("\nappend error [%ls] %u\n\n", mArchive.getErrorStr().p(), mArchive.getError());

	// close
	mArchive.close(mFileSystemType == 1);

	if(progress.mReturn)
	{
		// close window
		progress.closeWindow();
		// SystemMessage();

		if(mFileSystemType == 2)
		{
			mArchive.open(mArchiveName);
			statusArchive();
		}

		removeFile(mArchiveBackup);
	}
	else // cancel, remove archive
	{
		if(mFileSystemType == 1)
			removeFile(mArchiveName);
		else
		{
			// restore backup
			backupArchive(1);
			mArchive.open(mArchiveName);
		}
	}

	// enable main window
	mWindow->enable(1);

	_printf("# created archive %u\n", mReturn);

	return mReturn;
}

// user cancel unpack return -1
int FileManager::extractArchive()
{
	if(!mArchive.createLog(getLogName()))
	{
		_line;
		return 0;
	}

	// file paths
	byte relPath = 0;

	if((mUnpackType & 3) == 1) // from dialog
		relPath = mArchiverSettings.filePath;
	else if((mUnpackType & 3) == 2) // force relative
		relPath = 1;

	// directory
	mArchive.setValue((uint_t) mArchiveDirectory.p(), 10);

	// save path
	mArchive.setValue((uint_t) relPath != 0, 11);

	uint skipLen = 0;
	if(mArchiveLevel and relPath == 1) // relative path
		skipLen = mAddressArchive.length();
	mArchive.setValue((uint_t) skipLen, 17); // skip file name len

	// set callback event
	mArchive.setValue((uint_t) EventCallbackImpl, 8);

	// progress bar window
	ProgressWidget progress;
	progress.create(mUnpackType & 0x80 ? 2 : 1);

	// unpack files
	void* filesUnp = 0;
	if(mUnpackType & 4)
		filesUnp = &mArchiveFiles;

	// extract
	int mReturn = mArchive.extract(filesUnp);

	// mWindow->activate();

	if(progress.mReturn)
	{
		// close window
		progress.closeWindow();
	}
	else // cancel
	{
		mReturn = -1;
		if(mFileSystemType == 1)
			mArchive.close(1);
	}

	// enable main window
	mWindow->enable(1);
	mUnpackType = 0;

	_printf("# extracted archive files %u\n", mReturn);

	return mReturn;
}

bool FileManager::prepareExtract()
{
	// query password
	if(!mArchive.checkCrypt())
	{
		mArchiveName = mArchive.getName();
		if(!queryPassword(1))
			return 0;
	}
	// read list
	mArchiveFiles.clear();
	mListIndex = -1;
	while(getListSelect())
	{
		if(mItemType == 1) // dir
		{
			if(mArchiveLevel)
				mBuffer.format("%s%s\\*", mAddressArchive.p(), mFolders[mItemIndex].p());
			else
				mBuffer.format("%s\\*", mFolders[mItemIndex].p());
			mArchiveFiles.push_back(mBuffer);
		}
		else if(mItemType == 2) // file
		{
			if(mArchiveLevel)
				mBuffer.format("%s%s", mAddressArchive.p(), mFiles[mItemIndex].p());
			else
				mBuffer = mFiles[mItemIndex];
			mArchiveFiles.push_back(mBuffer);
		}
	}
	return 1;
}

bool FileManager::renameArchive()
{
	mListIndex = -1;
	getListSelect();
	int nItem = mListIndex;

	// source -> destination
	if(mItemType == 1)
		mBuffer.format("%s%s\\", mAddressArchive.p(), mItemString);
	else
		mBuffer.format("%s%s", mAddressArchive.p(), mItemString);
	mBufferw.format("%s%s", mAddressArchive.p(), mAddressEdit.p()); // new name

	if((mItemType == 1 or mItemType == 2) and fileExistArchive(mBufferw, 0))
		return 0;

	mArchiveFiles.clear();
	mArchiveFiles.push_back(mBuffer);
	mArchiveFiles.push_back(mBufferw);

	if(!mArchive.createLog(getLogName()))
		return 0;

	int mReturn = mArchive.rename(mArchiveFiles);

	updateFileList();
	mgFileList->selectItem(nItem);

	_printf("archive files renamed %u\n", mReturn);
	return mReturn;
}

bool FileManager::deleteArchive()
{
	mArchiveFiles.clear();
	mListIndex = -1;
	int nItem = -1; // first
	while(getListSelect())
	{
		if(mItemType == 3) continue;
		if(nItem == -1) nItem = mListIndex;
		if(mItemType == 1)
		{
			mBuffer.format("%s%s\\*", mAddressArchive.p(), mItemString);
		}
		else if(mItemType == 2)
		{
			mBuffer.format("%s%s", mAddressArchive.p(), mItemString);
		}
		mArchiveFiles.push_back(mBuffer);
	}

	// query password
	if(!mArchive.checkCrypt())
	{
		mArchiveName = mArchive.getName();
		if(!queryPassword(1))
			return 0;
	}

	if(!mArchive.createLog(getLogName()))
	{
		_line;
		return 0;
	}

	// set callback event
	mArchive.setValue((uint_t) EventCallbackImpl, 8);

	// backup
	backupArchive(0);

	// progress bar window
	ProgressWidget progress;
	progress.create(0);

	int mReturn = mArchive.remove(mArchiveFiles);

	// delete archive
	byte deleteArchive = 0;
	if(!mReturn and mArchive.getError() == 4)
		deleteArchive = 1;

	if(progress.mReturn)
	{
		// close window
		progress.closeWindow();
		// SystemMessage();
		removeFile(mArchiveBackup);
	}
	else // cancel, restore archive
	{
		if(!deleteArchive)
		{
			mArchive.close();
			backupArchive(1);
			mArchive.open(mArchiveName);
		}
	}
	if(deleteArchive)
	{
		mBuffer = mArchive.getName();
		closeArchive();
		removeFile(mBuffer);
		_printf("# deleted archive [%ls]\n", mBuffer.p());
		// update address, remove archive name
		mAddressCurrent = mBuffer;
		mAddressCurrent.trim(0x5C, 1);
		_printf("# deleted address [%ls]\n", mAddressCurrent.p());
		nItem = 0;
	}
	else
	{
		// not exist address in archive
		if(mArchiveLevel and !fileExistArchive(mAddressArchive, 0))
		{
			deleteArchive = 1;
			nItem = 0;
			_printf("# address not [%ls]\n", mAddressArchive.p());
			while(!fileExistArchive(mAddressArchive, 0))
			{
				// rem '\'
				mAddressArchive.resize(mAddressArchive.length()-1);
				int bs = mAddressArchive.findr(0x5C);
				if(bs == -1)
				{
					mAddressArchive.clear();
					break;
				}
				mAddressArchive.resize(bs+1);
			}
			if(mAddressArchive.length())
				mAddressCurrent.format("%s\\%s", mArchive.getName().p(), mAddressArchive.p());
			else
				mAddressCurrent = mArchive.getName();
			mAddressCurrent.resize(mAddressCurrent.length()-1);
		}
	}

	// enable main window
	mWindow->enable(1);
	if(deleteArchive)
		openFolder();
	else
		updateFileList();

	if(!deleteArchive)
	{
		int count = mgFileList->getItemCount(0);
		if(nItem >= count)
			nItem = count - 1;
	}

	mgFileList->selectItem(nItem);

	_printf("# deleted archive files %u\n", mReturn);

	return 1;
}

bool FileManager::testArchive()
{
	mReturn = 0;
	if(mFileSystemType == 1)
	{
		uint nCount[3] = {0};
		mListIndex = -1;
		while(getListSelect())
		{
			if(mItemType != 2) continue;
			mArchiveName.format("%s\\%s", mAddressCurrent.p(), mItemString);
			mReturn = mArchive.open(mArchiveName, 1);
			if(!mReturn)
			{
				if(mArchive.getError() == 4)
				{
					nCount[0]++;
					if(!queryPassword(0))
					{
						nCount[2]++;
						continue;
					}
				}
				else
					continue;
			}
			else
				nCount[0]++;
			if(!mDirectoryUnpack.length())
			{
				mBuffer = "dla.";
				generateFileName(mDirectoryUnpack, mBuffer, 1, 6);
				mDirectoryUnpack.update();
			}
			mArchiveDirectory = mDirectoryUnpack;
			mUnpackType = 0x80;
			mReturn = extractArchive();
			if(mReturn == -1) // cancel
				return 0;
			if(mReturn)
				nCount[1]++; // unpacked
			else if(!mReturn)
			{
				nCount[2]++; // errors
				_printf("error [%ls] %u\n", mArchive.getErrorStr(), mArchive.getError());
				// Archive %s tested with errors!
				mBuffer.format(mLocalisation[118].p(), mItemString);
				messageBox(mBuffer, mLocalisation[91], MB_OK | MB_ICONHAND);
			}
			mArchive.close(1);
		}
		// archives not found
		if(nCount[0] == 0)
		{
			mReturn = 0;
			// Archives not found!
			messageBox(mLocalisation[117], mLocalisation[91], MB_OK | MB_ICONWARNING);
		}
		else if(nCount[2] == 0)
			// Testing successfully completed
			messageBox(mLocalisation[119], mLocalisation[91], MB_OK | MB_ICONASTERISK);
	}
	else // inside
	{
		// query password
		mArchiveName = mArchive.getName();
		if(!mArchive.checkCrypt())
		{
			if(!queryPassword(1))
				return 0;
		}
		if(!mDirectoryUnpack.length())
		{
			mBuffer = "dla.";
			generateFileName(mDirectoryUnpack, mBuffer, 1, 6);
			mDirectoryUnpack.update();
		}
		mArchiveDirectory = mDirectoryUnpack;

		mUnpackType = 0x80;
		mReturn = extractArchive();
		mBufferw.assign(mArchiveName).substrxs(0x5C, 1, 1);
		if(mReturn)
		{
			// Testing successfully completed
			messageBox(mLocalisation[119], mLocalisation[91], MB_OK | MB_ICONASTERISK);
		}
		else if(!mReturn)
		{
			_printf("error [%ls] %u\n", mArchive.getErrorStr(), mArchive.getError());
			// Archive %s tested with errors!
			mBuffer.format(mLocalisation[118].p(), mBufferw.p());
			messageBox(mBuffer, mLocalisation[91], MB_OK | MB_ICONHAND);
		}
		mArchive.close(0);
		mArchive.open(mArchiveName);
		statusArchive();
	}
	if(mDirectoryUnpack.length())
	{
		removeFileShell(mDirectoryUnpack);
		mDirectoryUnpack.clear();
	}
	return mReturn;
}

int FileManager::openArchive()
{
	mReturn = 0;
	byte open = 0;
	// save pass
	if(mFileSystemType == 2)
	{
		open = 1;
		// archive name
		mArchiveDirectory = mArchive.getName();
		if(mArchive.getValue(5) and mArchive.checkCrypt())
		{
			open = 2;
			mArchiveBackup = reinterpret_cast<char*>(mArchive.getValue(10)); // pass
		}
	}

	mArchive.close(1);
	mReturn = mArchive.open(mArchiveName);

	// password
	if(!mReturn and mArchive.getError() == 4)
	{
		if(!queryPassword(0))
			mReturn = -1;
	}

	if(mReturn != 1)
	{
		mArchive.close(1);
		// reopen previous archive
		if(open)
		{
			if(open == 2)
				mArchive.setValue((uint_t) mArchiveBackup.p(), 6);
			int retx = mArchive.open(mArchiveDirectory);
			assert(retx);
		}
		return mReturn;
	}

	if(mFileSystemType == 1) // CRC-32
		mgFileList->setColumn(mLocalisation[33].p(), 4, 70, 0, 0);
	mArchiveLevel = 0;
	mFileSystemType = 2;
	mAddressArchive.clear();
	mArchiveFileList = (Array<ArchiveFileList*>*)mArchive.getValue(0);

	// archive status
	statusArchive();

	return mReturn;
}

bool FileManager::closeArchive()
{
	mArchive.close(1);
	mFileSystemType = 1;
	mgFileList->deleteColumn(4); // crc
	forn(2) mStatusbar->setText(i+1, "");
	mArchiveLevel = 0;
	mAddressArchive.clear();
	return 1;
}

bool FileManager::closeArchiveMenu()
{
	if(mFileSystemType == 1) return 0;
	closeArchive();
	String arcName = mArchive.getName();
	mAddressEdit = arcName.substrx(0x5C, 1, 0);
	mAddressUp   = arcName.substrx(0x5C, 1, 1);
	return addressEdited();
}

bool FileManager::openArchiveDialog()
{
	if(mFileSystemType == 1)
		mArchiveDirectory = mAddressCurrent;
	else
	{
		mArchiveDirectory = mArchive.getName();
		mArchiveDirectory.trim(0x5C, 1);
	}
	String maskFile(FM->mLocalisation[56]);
	maskFile.append(L" (*.*)\0*.*\0\0", 15);
	mBuffer.clear();
	OPENFILENAME ofn;
	menset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mWindow->getHandle();
	ofn.lpstrFile = mBuffer.p();
	ofn.nMaxFile = mBuffer.capacity();
	ofn.lpstrFilter = maskFile.p();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = mArchiveDirectory.p();
	ofn.Flags = OFN_EXPLORER | OFN_NOVALIDATE | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = FM->mLocalisation[55].p(); // Select archive location
	mReturn = GetOpenFileName(&ofn);
	FM->mKeyboard.setKeyState(VK_ESCAPE, 1);
	FM->mKeyboard.setKeyState(VK_RETURN, 1);
	if(mReturn)
	{
		mAddressEdit = ofn.lpstrFile;
		addressEdited();
	}
	return 1;
}

bool FileManager::openFileArchive()
{
	// extract file from opened archive, launch
	mBuffer.format("%s%s", mAddressArchive.p(), mItemString);
	mArchiveFiles.clear();
	mArchiveFiles.push_back(mBuffer);
	mUnpackType = 4;
	if(!mDirectoryUnpack.length())
	{
		mBuffer = "dla.";
		generateFileName(mDirectoryUnpack, mBuffer, 1, 6);
		mDirectoryUnpack.update();
	}
	mArchiveDirectory = mDirectoryUnpack;
	// query password
	if(!mArchive.checkCrypt())
	{
		mArchiveName = mArchive.getName();
		if(!queryPassword(1))
			return 0;
	}
	mReturn = extractArchive();
	if(mReturn)
	{
		mBuffer.format("%s\\%s", mDirectoryUnpack.p(), mItemString);
		mReturn = (int) ShellExecute(0, L"open", mBuffer.p(), 0, mDirectoryUnpack.p(), SW_SHOW);
		if(mReturn <= 32)
		{
			mReturn = 0;
			_printf("# file not launch %d [%ls]\n", mReturn, mBuffer.p());
		}
	}
	else
	{
		_printf("unpack error [%ls] %u\n", mArchive.getErrorStr().p(), mArchive.getError());
		mBufferw.assign(mArchive.getName()).substrxs(0x5C, 1, 1);
		// Unarchiving %s completed with errors!
		mBuffer.format(mLocalisation[116].p(), mBufferw.p());
		messageBox(mBuffer, mLocalisation[90], MB_OK | MB_ICONWARNING);
	}
	return 1;
}

bool FileManager::copyClipboardArchive()
{
	mReturn = prepareExtract();
	if(!mReturn) return 0;

	if(!mDirectoryUnpack.length())
	{
		mBuffer = "dla.";
		generateFileName(mDirectoryUnpack, mBuffer, 1, 6);
		mDirectoryUnpack.update();
	}
	else
	{
		// recreate
		removeFileShell(mDirectoryUnpack);
		tas::createFolder(mDirectoryUnpack, 1);
	}
	mArchiveDirectory = mDirectoryUnpack;

	mUnpackType = 6;
	mReturn = extractArchive();
	if(!mReturn)
	{
		_printf("unpack error [%ls] %u\n", mArchive.getErrorStr(), mArchive.getError());
		mBufferw.assign(mArchive.getName()).substrxs(0x5C, 1, 1);
		// Unarchiving %s completed with errors!
		mBuffer.format(mLocalisation[116].p(), mBufferw.p());
		messageBox(mBuffer, mLocalisation[90], MB_OK | MB_ICONWARNING);
		return 0;
	}

	mReturn = copyClipboardImpl(1);
	if(!mReturn) return 0;
	return 1;
}

bool FileManager::searchArchiveFiles()
{
	_printf("# search archive file list [%ls] %u\n", mAddressArchive.p(), mArchiveLevel);
	uint addressLen = 0;

	if(mArchiveLevel != 0)
		addressLen = mAddressArchive.length();

	if(mArchiveListCurrent.capacity() == 0)
		mArchiveListCurrent.reserve(32);
	mArchiveListCurrent.resize(0);

	forn(mArchiveFileList->size())
	{
		ArchiveFileList* list = (*mArchiveFileList)[i];
		String file = list->name;

		// parsing files root
		if(mArchiveLevel != 0)
		{
			if(mAddressArchive.compare(file, 0, addressLen) == 0)
				file.substrs(addressLen);
			else
				continue;
		}

		// build folder file list
		uint bs = file.find(0x5C);
		if(bs != MAX_UINT32)
		{
			// directory
			mBuffer.assign(file, 0, bs);
			bool compare = 0;
			form(u, mFolders.size())
			{
				if(mBuffer == mFolders[u])
				{
					compare = 1;
					break;
				}
			}
			if(!compare)
				mFolders.push_back(mBuffer);
		}
		else
		{
			mFiles.push_back(file);
			mArchiveListCurrent.push_back(list);
		}
	}

	return 1;
}

bool FileManager::updateFileListArchive()
{
	uint addressLen = 0;
	if(mArchiveLevel != 0)
		addressLen = mAddressArchive.length();

	Array<ArchiveFileList*> mArchiveListUpdate;
	mArchiveListUpdate.reserve(mArchiveListCurrent.size() + 2);

	forn(mArchiveListCurrent.size())
	{
		ArchiveFileList* list = mArchiveListCurrent[i];
		String file = list->name;
		if(mArchiveLevel != 0)
			file.substrs(addressLen);
		form(u, mFiles.size())
		{
			if(mFiles[u] == file)
			{
				mArchiveListUpdate.push_back(mArchiveListCurrent[i]);
				break;
			}
		}
	}
	mArchiveListCurrent.resize(0);
	mArchiveListCurrent = mArchiveListUpdate;
	return 1;
}

bool FileManager::fileExistArchive(String& fileIn, byte copy)
{
	String fileInw = fileIn;
	String filew;
	fileInw.tolower();
	uint len = fileIn.length();
	forn(mArchiveFileList->size())
	{
		String& file = (*mArchiveFileList)[i]->name;
		filew = file;
		filew.tolower();
		if(fileInw.compare(filew, 0, len) == 0)
		{
			if(copy) fileIn.assign(file, 0, len);
			return 1;
		}
	}
	return 0;
}

bool FileManager::backupArchive(byte mode)
{
	if(!mode)
	{
		// backup archive
		mArchiveName = mArchive.getName();
		mBufferw.format("%ls.", mArchiveName.p());
		generateFileName(mArchiveBackup, mBufferw, 0, 6, 0);
		CopyFile(mArchiveName.p(), mArchiveBackup.p(), 0);
		hideFile(mArchiveBackup.p());
	}
	else
	{
		// restore backup
		int mReturn = removeFile(mArchiveName);
		mReturn = tas::renameFile(mArchiveBackup, mArchiveName);
		hideFile(mArchiveName.p(), 0);
	}
	return 1;
}

bool FileManager::statusArchive()
{
	// mBuffer = "Method ";
	mBuffer.format("%s ", mLocalisation[73].p());

	byte archiveMethod = mArchive.getValue(2);
	byte archiveRatio = mArchive.getValue(4);
	byte archiveCrypt = mArchive.getValue(5);

	// method
	switch(archiveMethod)
	{
	case 0:
		mBuffer += mLocalisation[88].p(); // "None";
		break;
	case 1:
		mBuffer += "LZRE";
		break;
	case 2:
		mBuffer += "CM";
		break;
	}

	mStatusbar->setText(1, mBuffer);

	// crypt
	mBuffer.format("%s ", mLocalisation[81].p());
	// mBuffer = "Crypt ";
	switch(archiveCrypt)
	{
	case 0:
		mBuffer += mLocalisation[88].p(); // "None";
		break;
	case 1:
		mBuffer += mLocalisation[82].p(); // "Names";
		break;
	case 2:
		mBuffer += mLocalisation[83].p(); // "Data";
		break;
	case 3:
		mBuffer += mLocalisation[84].p(); // "All";
		break;
	}

	mStatusbar->setText(2, mBuffer);

	return 1;
}

bool FileManager::queryPassword(byte type)
{
	if(!type)
		mArchive.close(1);

	// extract name
	mBuffer = mArchiveName.substrx(0x5C, 1, 1);

	PasswordWidget passwordFrame;
	mReturn = passwordFrame.create(mBuffer);
	if(!mReturn)
		return 0;

	mArchive.setValue((uint_t) mPassword.p(), 6);
	if(!type)
		mReturn = mArchive.open(mArchiveName, 1);
	else
		mReturn = mArchive.checkCrypt();

	if(!mReturn)
	{
		if(!type)
			mArchive.close(1);
		// Password incomplete!
		messageBox(mLocalisation[120], PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
		return 0;
	}

	return 1;
}

}