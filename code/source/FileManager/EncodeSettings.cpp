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
#include <FileManager/EncodeSettingsDef.h>
#include <FileManager/EncodeSettings.h>
#include <Common/FileAccess.h>
#include <Common/StringLib.h>
#include <Common/Math.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>
#include <Widget/Label.h>

namespace tas
{

EncodeSettings* EncodeSettings::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

EncodeParams::EncodeParams()
{
	reset();
}

EncodeParams::~EncodeParams()
{
}

void EncodeParams::reset()
{
	forn(12) operator[](i) = -1;
}

uint& EncodeParams::operator[](int i)
{
	return *(&level + i);
}

EncodeSettings::EncodeSettings()
{
	mPtr = this;
	mWindow = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(340, 400);
	mCompressionLevel = 3;
	mCompressionMethod = 0;
	mSelectId = 0;
	mDropdown = 0;
	mReturn = 0;
	mArchiveName = 0;
	mParameters = 0;
	mPassword = 0;
	mPreproc = 0;
	mCrypt = 0;
	mPasswordShow = 0;
	forn(6) mSettings[i] = 0;
	mBuffer.reserve(BUFFER_SIZE);
	mArchiveDirectory.reserve(BUFFER_SIZE);
	mArchiveNames.reserve(BUFFER_SIZE);
}

EncodeSettings::~EncodeSettings()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool EncodeSettings::create(const String& arcDir, const String& arcName)
{
	mArchiveDirectory = arcDir;
	mArchiveNames = arcName;

	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[35], 2, mWindowSize, mWindowPos,
	                     WidgetCallbackImpl, IdleCallbackImpl);

	uint windowProps = 12;

	// create widgets
	createItems();

	// create main window
	mWindow->createEx(windowProps, 1);

	// initialise widgets
	initialise();

	// main loop
	mWindow->run();

	FM->mWindow->enable(1);
	FM->mWindow->activate();

	return mReturn;
}

bool EncodeSettings::createItems()
{
	// labels 9
	/* char* labelNames[] =
	{
		"Archive", "Compression level", "Compression method", "Dictionary size",
		"Match length maximum", "Match length minimum", "Threads number",
		"Parameters", "Password"
	}; */

	Vector2i labelPos(10, 20), labelSize(200, 20),
	         editPos (10, 42), editSize (300, 20),
	         comboPos(10, 80), comboSize(120, 20);

	uint editId  = 16;
	uint comboId = 19;
	uint labelId = 25;

	// labels 9
	forn(9)
	{
		Label* label = new Label(mWindow, FM->mLocalisation[36 + i], labelId++, 0, labelSize, labelPos);
		if(!i)
		{
			labelPos.y += 64;
			labelPos.x += 150;
		}
		else if(i < 7)
			labelPos.y += 35;
		if(i == 6) // before parameters
			labelPos.x = 10;
		if(i == 7) // before password
			labelPos.x = 160;
	}

	// archive name, parameters, password
	editSize.x = mWindowSize.x - 20;
	bool pass = 0;
	forn(3)
	{
		if(i == 2)
		{
			pass = 1;
			if(FM->mArchiverSettings.showPass != 0xFF)
				pass = !FM->mArchiverSettings.showPass;
		}

		Edit* edit = new Edit(mWindow, "", editId++, pass, editSize, editPos);
		if(!i)
		{
			mArchiveName = edit;
			editPos.y = 314;
			editSize.x = 120;
		}
		else
		{
			editPos.x = 160;
			editSize.x = 170;
		}
		if(i == 1)
			mParameters = edit;
		if(i == 2)
			mPassword = edit;
	}

	// buttons file dialog '...', OK, Cancel
	Vector2i buttonSize(30, 20);
	Vector2i buttonPos(mWindowSize.x - buttonSize.x - 10, 15);
	forn(3)
	{
		Button* button = new Button(mWindow, FM->mLocalisation[57 + i], ID_FILE_DLG + i, buttonSize, buttonPos);
		if(!i)
		{
			buttonSize = FM->mButtonSize;
			buttonPos.x = mWindowSize.x - (buttonSize.x * 2) - 20;
			buttonPos.y = mWindowSize.y - buttonSize.y - 10;
		}
		else
			buttonPos.x += buttonSize.x + 10;
	}

	// checkbox bigraph preproc, encrypt file names, show password
	Vector2i cbSize(14, 14);
	Vector2i cbPos(316, 296);
	uint cbPosv[] = {116, 295, 316};
	Checkbox** checkboxPtr = &mPreproc;
	forn(3)
	{
		cbPos.x = cbPosv[i];
		Checkbox* checkbox = new Checkbox(mWindow, "", ID_PREPROC + i, cbSize, cbPos);
		checkboxPtr[i] = checkbox;
	}

	// compression settings
	forn(6)
	{
		mSettings[i] = new Combobox(mWindow, "", comboId++, 0, comboSize, comboPos);
		comboPos.y += 35;
	}

	return 1;
}

bool EncodeSettings::initialise()
{
	forn(24)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		label->setFont(FM->mFontSystem);
	}
	// char* tooltips[] = { "File dialog", "Bigraph preprocessing", "Encrypt file names", "Show password" };
	uint id = 10;
	forn(4)
	{
		Widget* item = mWindow->findWidgetById(id++);
		item->createTooltip(FM->mLocalisation[51 + i]);
		if(!i) id = 13;
	}
	mArchiveName->setText(mArchiveNames);
	initialiseSettings(3);
	return 1;
}

bool EncodeSettings::initialiseSettings(byte level)
{
	// initialise compression levels
	if(level & 1)
	{
		if(FM->mArchiverSettings.level != 0xFF)
		{
			mCompressionLevel = FM->mArchiverSettings.level;
			if(FM->mArchiverSettings.method != 0xFF)
				mCompressionMethod = FM->mArchiverSettings.method;
		}
		// char* cmLevel[] = {"Store", "Fastest", "Fast", "Normal", "Quality", "Ultra"};
		forn(6) mSettings[0]->appendString(FM->mLocalisation[45 + i]);
		mSettings[0]->setCurrentItem(mCompressionLevel);
	}

	// initialise from compression methods to end
	if(mCompressionLevel)
	{
		if(level & 2)
		{
			// compression methods
			char* cmMethod[] = {"LZRE", "CM"};
			forn(2) mSettings[1]->appendString(cmMethod[i]);
			mSettings[1]->setCurrentItem(mCompressionMethod);
		}
		// LZRE
		if(mCompressionMethod == 0)
		{
			// dictionary size [64 kb, 256 mb]
			uint dictSize = 0;
			forn(13)
			{
				if(i < 4)
				{
					dictSize = 64 << i;
					mBuffer.format("%u KB", dictSize);
					mSettings[2]->appendString(mBuffer);
				}
				else
				{
					dictSize = 1 << (i - 4);
					mBuffer.format("%u MB", dictSize);
					mSettings[2]->appendString(mBuffer);
				}
			}

			// match length maximum [32, 1024]
			uint matchLength = 0;
			forn(6)
			{
				matchLength = 32 << i;
				mBuffer.format("%u", matchLength);
				mSettings[3]->appendString(mBuffer);
			}

			// match length minimum [2, 6]
			forn(5)
			{
				matchLength = 2 + i;
				mBuffer.format("%u", matchLength);
				mSettings[4]->appendString(mBuffer);
			}

			// threads 2
			forn(2)
			{
				mBuffer.format("%u", i + 1);
				mSettings[5]->appendString(mBuffer);
			}
			mSettings[5]->setCurrentItem(1); // 2
		}
		// CM
		else if(mCompressionMethod == 1)
		{
			// dictionary size [1 mb, 16 mb]
			uint dictSize = 0;
			forn(5)
			{
				dictSize = 1 << i;
				mBuffer.format("%u MB", dictSize);
				mSettings[2]->appendString(mBuffer);
			}

			// match length maximum [4, 8]
			uint matchLength = 0;
			forn(5)
			{
				matchLength = 4 + i;
				mBuffer.format("%u", matchLength);
				mSettings[3]->appendString(mBuffer);
			}
		}
	}

	// continue initialise, set state from main archiver settings
	if(level & 1)
	{
		if(FM->mArchiverSettings.level != 0xFF)
		{
			if(FM->mArchiverSettings.level != 0)
			{
				forn(6)
				{
					if(FM->mArchiverSettings[i] != 0xFF)
						mSettings[i]->setCurrentItem(FM->mArchiverSettings[i]);
				}
				if(FM->mArchiverSettings.bigraph != 0xFF)
					mPreproc->setState(FM->mArchiverSettings.bigraph);
			}
		}
		else
		{
			mSelectId = ID_COMPRESS_LEVEL;
			updateSettings();
		}
		if(FM->mArchiverSettings.cryptFile != 0xFF)
			mCrypt->setState(FM->mArchiverSettings.cryptFile);
		if(FM->mArchiverSettings.showPass != 0xFF)
			mPasswordShow->setState(FM->mArchiverSettings.showPass);
	}

	// set state bigraph, threads
	if(level & 4)
	{
		if(FM->mArchiverSettings.bigraph != 0xFF)
			mPreproc->setState(FM->mArchiverSettings.bigraph);
		if(FM->mArchiverSettings.threads != 0xFF)
			mSettings[5]->setCurrentItem(FM->mArchiverSettings.threads);
	}

	return 1;
}

bool EncodeSettings::updateSettings()
{
	_printf("update settings %d\n", mSelectId);

	if(mSelectId == ID_COMPRESS_LEVEL)
	{
		int select = mSettings[0]->getCurrentItem();
		_printf("  select level %d\n", select);
		// level 'store', clear 5 controls
		if(select == 0)
		{
			FM->mArchiverSettings.bigraph = mPreproc->getState();
			FM->mArchiverSettings.threads = mSettings[5]->getCurrentItem();
			forn(5) mSettings[i+1]->deleteAllItems();
			mPreproc->setState(0);
		}
		else
		{
			// previous level 'store'
			if(mCompressionLevel == 0)
			{
				mCompressionLevel = select;
				initialiseSettings(6);
			}
			if(mCompressionMethod == 0) // LZRE
			{
				byte dicts[] = {0, 4, 8, 9, 10};
				mSettings[2]->setCurrentItem(dicts[select - 1]);
				mSettings[3]->setCurrentItem(clamp(select, 3, 1)); // match max [64, 256]
				mSettings[4]->setCurrentItem(2); // match min 4
			}
			else // CM
			{
				mSettings[2]->setCurrentItem(select - 1);
				mSettings[3]->setCurrentItem(select - 1);
			}
		}
		mCompressionLevel = select;
	}
	else if(mSelectId == ID_COMPRESS_METHOD)
	{
		int select = mSettings[1]->getCurrentItem();
		forn(4) mSettings[i+2]->deleteAllItems();
		mCompressionMethod = select;
		initialiseSettings(0);
		mSelectId = ID_COMPRESS_LEVEL;
		updateSettings();
	}

	mSelectId = 0;
	return 1;
}

bool EncodeSettings::saveSettings()
{
	_printf("save settings\n");
	mReturn = 1;
	uint empty = -1;

	mArchiveName->getText(mBuffer);
	mBuffer.update();
	if(mBuffer.length())
		mArchiveNames = mBuffer;

	forn(6) FM->mArchiverSettings[i] = mSettings[i]->getCurrentItem();
	FM->mArchiverSettings.bigraph = mPreproc->getState();
	FM->mArchiverSettings.cryptFile = mCrypt->getState();
	FM->mArchiverSettings.showPass = mPasswordShow->getState();

	// get command line parameters string, parsing
	mParameters->getText(mBuffer);
	mBuffer.update();
	parsingParameters();

	// level
	if(mEncodeParams.level == empty)
	{
		mEncodeParams.level = mSettings[0]->getCurrentItem();
		if(mEncodeParams.level)
			mEncodeParams.level = mEncodeParams.level * 2 - 1;
	}

	// crypt
	mPassword->getText(FM->mPassword);
	FM->mPassword.update();
	if(mEncodeParams.crypt == empty)
	{
		mEncodeParams.crypt = 0;
		if(FM->mPassword.length())
		{
			mEncodeParams.crypt = 2; // data crypt
			if(mCrypt->getState())
				mEncodeParams.crypt = 3; // with file name
		}
	}

	if(mEncodeParams.level == 0)
		return 1;

	// method
	if(mEncodeParams.method == empty)
		mEncodeParams.method = mSettings[1]->getCurrentItem() + 1;

	if(mEncodeParams.method == 0)
	{
		mEncodeParams.level = 0;
		return 1;
	}

	// dictionary
	if(mEncodeParams.dictSize == empty)
	{
		if(mEncodeParams.method == 1) // LZRE
			mEncodeParams.dictSize = 1 << 16 << mSettings[2]->getCurrentItem();
		else if(mEncodeParams.method == 2) // CM
			mEncodeParams.dictSize = 1 << 20 << mSettings[2]->getCurrentItem();
	}

	// match max
	if(mEncodeParams.matchMax == empty)
	{
		if(mEncodeParams.method == 1)
			mEncodeParams.matchMax = 32 <<  mSettings[3]->getCurrentItem();
		else if(mEncodeParams.method == 2)
			mEncodeParams.matchMax = mSettings[3]->getCurrentItem() + 4;
	}

	// match min
	if(mEncodeParams.matchMin == empty)
	{
		if(mEncodeParams.method == 1)
			mEncodeParams.matchMin = mSettings[4]->getCurrentItem() + 2;
	}

	// threads
	if(mEncodeParams.threads == empty)
	{
		if(mEncodeParams.method == 1)
			mEncodeParams.threads = mSettings[5]->getCurrentItem() + 1;
	}

	// bigraph
	if(mEncodeParams.bigraph == empty)
	{
		mEncodeParams.bigraph = mPreproc->getState();
	}

	return 1;
}

bool EncodeSettings::fileDialog()
{
	// All files (*.*)\0*.*\0\0
	mBuffer = FM->mLocalisation[56];
	mBuffer.append(L" (*.*)\0*.*\0\0", 15);
	OPENFILENAME ofn;
	menset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mWindow->getHandle();
	ofn.lpstrFile = mArchiveNames.p();
	ofn.nMaxFile = mArchiveNames.capacity();
	ofn.lpstrFilter = mBuffer.p();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = mArchiveDirectory.p();
	ofn.Flags = OFN_EXPLORER | OFN_NOVALIDATE;
	ofn.lpstrTitle = FM->mLocalisation[55].p(); // Select archive location
	if(GetOpenFileName(&ofn))
	{
		mArchiveDirectory = ofn.lpstrFile;
		int bs = mArchiveDirectory.findr(0x5C);
		mArchiveNames.assign(mArchiveDirectory, bs + 1);
		mArchiveDirectory.resize(bs);
		mArchiveName->setText(mArchiveNames);
	}
	FM->mKeyboard.setKeyState(VK_ESCAPE, 1);
	FM->mKeyboard.setKeyState(VK_RETURN, 1);
	return 1;
}

bool EncodeSettings::switchPassword()
{
	mPassword->recreate(!mPasswordShow->getState());
	mPassword->setFont(FM->mFontSystem);
	return 1;
}

bool EncodeSettings::parsingParameters()
{
	mEncodeParams.reset();

	uint len = mBuffer.length();
	if(!len) return 0;
	_printf("# parameters %ls\n", mBuffer.p());
	uint posa; // non space char
	uint posb; // space char
	byte m = 0; // mode, 0 non space, 1 space, 2 find
	uint i = 0;
	String arg;
	arg.reserve(512);

#define CMP(s,n) (arg.compare(L##s, n) == 0)

	while(1)
	{
		// non space char
		if(!m and mBuffer[i] != 32)
		{
			posa = i;
			m = 1;
		}
		else if(m and (mBuffer[i] == 32 or i == len - 1))
		{
			posb = i;
			if(i == len - 1)
				posb++;
			m = 2;
		}
		// parsing parameters
		if(m == 2)
		{
			arg.assign(mBuffer, posa, posb - posa);
			_printf("param [%ls]\n", arg.p());

			if(CMP("-ml", 3))
			{
				mEncodeParams.level = stdwtoi(arg.p() + 3);
				if(!range(mEncodeParams.level, 9, 1))
					mEncodeParams.level = -1;
			}

			else if(CMP("-mt", 3))
				mEncodeParams.threads = stdwtoi(arg.p() + 3);

			else if(CMP("-mw", 3))
				mEncodeParams.matchMax = stdwtoi(arg.p() + 3);

			else if(CMP("-mc", 3))
				mEncodeParams.matchCycles = stdwtoi(arg.p() + 3);

			else if(CMP("-mn", 3))
				mEncodeParams.matchMin = stdwtoi(arg.p() + 3);

			else if(CMP("-ms", 3))
				mEncodeParams.suffix = stdwtoi(arg.p() + 3);

			else if(CMP("-mr", 3))
				mEncodeParams.rolz = stdwtoi(arg.p() + 3);

			else if(CMP("-mx", 3))
				mEncodeParams.cmix = 1;

			else if(CMP("-cr", 3))
				mEncodeParams.crypt = stdwtoi(arg.p() + 3);

			else if(CMP("-bg", 3))
				mEncodeParams.bigraph = 1;

			else if(CMP("-md", 3))
			{
				String dictsz = arg.substr(3);
				byte lem = dictsz.length();
				if(dictsz[lem - 1] == 'k')
				{
					dictsz[lem - 1] = 0;
					mEncodeParams.dictSize = stdwtoi(dictsz.p()) * KB;
				}
				else if(dictsz[lem - 1] == 'm')
				{
					dictsz[lem - 1] = 0;
					mEncodeParams.dictSize = stdwtoi(dictsz.p()) * MB;
				}
				else
					mEncodeParams.dictSize = 1 << stdwtoi(dictsz.p());
			}

			else if(CMP("-m", 2))
			{
				mEncodeParams.method = stdwtoi(arg.p() + 2);
				if(mEncodeParams.method > 2)
					mEncodeParams.method = -1;
			}

			m = 0;
		}
		if(++i == len) break;
	}

	return 1;
}

int EncodeSettings::callback(Widget* widget, uint message, uint_t param)
{
	switch(message)
	{
	case EVM_CLICKED:
	{
		onButtons(param);
		break;
	}
	case EVM_CB_SELCHANGE:
	{
		mSelectId = param;
		setCommand(ENC_SETTING_SELECT);
		break;
	}
	case EVM_CB_DROP_SHOW:
	{
		mDropdown = 1;
		break;
	}
	case EVM_CB_DROP_CLOSE:
	{
		mDropdown = 0;
		FM->mKeyboard.setKeyState(VK_ESCAPE, 1);
		FM->mKeyboard.setKeyState(VK_RETURN, 1);
		break;
	}
	}
	return 1;
}

void EncodeSettings::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void EncodeSettings::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case FMC_EXIT:
			mWindow->close();
			break;
		case ENC_SETTING_SELECT:
			updateSettings();
			break;
		case ENC_SETTING_SAVE:
			saveSettings();
			break;
		case ENC_FILE_DIALOG:
			fileDialog();
			break;
		case ENC_PASSWORD:
			switchPassword();
			break;
		case ENC_PREPROC:
			if(mCompressionLevel == 0)
				mPreproc->setState(0);
			break;
		}
	}
	mCommands.resize(0);
}

void EncodeSettings::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		_printf("return dropdown %d\n", mDropdown);
		// save state
		if(!mDropdown)
		{
			setCommand(ENC_SETTING_SAVE);
			setCommand(FMC_EXIT);
		}
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		_printf("escape dropdown %d\n", mDropdown);
		if(!mDropdown)
			setCommand(FMC_EXIT);
	}
}

void EncodeSettings::onButtons(uint id)
{
	_printf("onButtons %u \n", id);
	switch(id)
	{
	case ID_OK:
	{
		setCommand(ENC_SETTING_SAVE);
		setCommand(FMC_EXIT);
		break;
	}
	case ID_CANCEL:
	{
		setCommand(FMC_EXIT);
		break;
	}
	case ID_FILE_DLG:
	{
		setCommand(ENC_FILE_DIALOG);
		break;
	}
	case ID_PASSWORD:
	{
		setCommand(ENC_PASSWORD);
		break;
	}
	case ID_PREPROC:
	{
		setCommand(ENC_PREPROC);
		break;
	}
	}
}

void EncodeSettings::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

EncodeSettings* EncodeSettings::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return EncodeSettings::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	EncodeSettings::ptr()->idle();
}

}