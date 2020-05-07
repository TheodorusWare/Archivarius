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
#include <FileManager/ArchiverOptions.h>
#include <Common/Registry.h>
#include <Common/String.h>
#include <Common/StringLib.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>
#include <Widget/Checkbox.h>
#include <Widget/Label.h>

namespace tas
{

#define ID_OK        10
#define ID_CANCEL    11

#define ID_ALIGN       12
#define ID_MAXIMIZED   13
#define ID_FULLROW     14
#define ID_LANGUAGE    15
#define ID_SYSTEM      16

#define CMD_SETTING_SAVE  19

#define SETTING FM->mArchiverSettings

ArchiverOptions* ArchiverOptions::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

ArchiverOptions::ArchiverOptions()
{
	mPtr = this;
	mWindow = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(240, 240);
	mReturn = 0;
	mBuffer.reserve(BUFFER_SIZE);
	mBufferw.reserve(BUFFER_SIZE);
	forn(4)
	{
		if(i < 3)
			mInitState[i] = 0;
		mCurState[i] = 0;
	}
	forn(5)
	{
		mOptionItems[i] = 0;
		if(i < 2) mSettings[i] = 0;
	}
}

ArchiverOptions::~ArchiverOptions()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool ArchiverOptions::create()
{
	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[4], 2, mWindowSize, mWindowPos,
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

	// apply settings after main window enable
	if(mReturn == 1)
		saveSettings(1);

	return mReturn;
}

bool ArchiverOptions::createItems()
{
	Vector2i cbPos(10, 10);
	Vector2i cbSize(160, 20);
	/* char* names[] =
	{
		"Window align center", "Window maximized", "Full row select",
		"Language", "System"
	}; */

	forn(3)
	{
		mOptionItems[i] = new Checkbox(mWindow, FM->mLocalisation[97 + i], ID_ALIGN + i, cbSize, cbPos);
		cbPos.y += 30;
	}

	cbPos.y = 114;
	cbPos.x += 135;
	forn(2)
	{
		Label* label = new Label(mWindow, FM->mLocalisation[100 + i], ID_LANGUAGE + i, 0, cbSize, cbPos);
		cbPos.y += 35;
	}

	// combobox language, system
	cbSize.x = 120;
	cbPos = Vector2i(10, 110);
	forn(2)
	{
		mSettings[i] = new Combobox(mWindow, "", 17 + i, 0, cbSize, cbPos);
		cbPos.y += 35;
	}

	// buttons OK, Cancel
	Vector2i buttonSize(FM->mButtonSize);
	Vector2i buttonPos;
	buttonPos.x = mWindowSize.x - (buttonSize.x * 2) - 20;
	buttonPos.y = mWindowSize.y - buttonSize.y - 10;
	forn(2)
	{
		Button* button = new Button(mWindow, FM->mLocalisation[58 + i], ID_OK + i, buttonSize, buttonPos);
		buttonPos.x += buttonSize.x + 10;
	}

	return 1;
}

bool ArchiverOptions::initialise()
{
	forn(9)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		label->setFont(FM->mFontSystem);
	}

	if(SETTING.windowAlign)
		mOptionItems[0]->setState(1);

	if(FM->mWindow->isMaximized())
		mOptionItems[1]->setState(1);

	if(SETTING.fullRowSelect)
		mOptionItems[2]->setState(1);

	forn(3)
	mInitState[i] = mOptionItems[i]->getState();

	// language en ru
	wchar_t* languages[] = {L"English", L"Russian"};
	forn(2) mSettings[0]->appendString(languages[i]);
	uint languageId = 0;
	if(FM->mLocalisationId == L"ru")
		languageId = 1;
	mSettings[0]->setCurrentItem(languageId);

	// system: none, integration, segregation
	int id[] = {88, 102, 103};
	forn(3) mSettings[1]->appendString(FM->mLocalisation[id[i]]);
	mSettings[1]->setCurrentItem(0);

	return 1;
}

bool ArchiverOptions::saveSettings(byte mode)
{
	// save
	if(!mode)
	{
		mReturn = 1;

		forn(3)
		mCurState[i] = mOptionItems[i]->getState();

		SETTING.windowAlign    = mOptionItems[0]->getState();
		SETTING.windowMaximize = mOptionItems[1]->getState();
		SETTING.fullRowSelect  = mOptionItems[2]->getState();

		// language
		int languageId = mSettings[0]->getCurrentItem();
		if(languageId == 0)
			FM->mLocalisationId = "en";
		else if(languageId == 1)
			FM->mLocalisationId = "ru";

		// system integration
		int systemSelect = mSettings[1]->getCurrentItem();
		if(systemSelect == 1)
			mCurState[3] = 1;
		else if(systemSelect == 2)
			FM->mSystemSegregation = 1;
	}
	else // apply
	{
		// maximize
		if(mCurState[1] != mInitState[1])
		{
			FM->mWindow->maximize(mCurState[1]);
		}

		// align center 1, max 0
		if(mCurState[0] != mInitState[0] and mCurState[0] and !mCurState[1])
		{
			FM->mWindow->alignByCenter();
		}

		// full row
		if(mCurState[2] != mInitState[2])
		{
			int style = 1;
			if(SETTING.fullRowSelect)
				style = 3;
			FM->mgFileList->setExtendedStyle(style);
		}

		// system integration (associate archive extension with program in registry)
		if(mCurState[3])
		{
			_printf("system integration \n");
			uint errors = 0;
			Registry registry;

			// .ext = program name
			int ret = registry.create(HKEY_CLASSES_ROOT, ARCHIVE_EXT);
			errors |= !ret ? 1 : 0;
			if(ret)
			{
				ret = registry.setValue("", REG_SZ, reinterpret_cast<byte*>(PROGRAM_TITLE), stwlen(PROGRAM_TITLE) * 2);
				errors |= !ret ? 2 : 0;
			}
			registry.close();

			// default icon from module name by index 0
			GetModuleFileName(0, mBuffer.p(), BUFFER_SIZE);
			mBuffer.update();
			ret = registry.create(HKEY_CLASSES_ROOT, "Archivarius\\DefaultIcon");
			errors |= !ret ? 4 : 0;
			if(ret)
			{
				mBufferw.format("%s,0", mBuffer.p());
				ret = registry.setValue("", REG_SZ, reinterpret_cast<byte*>(mBufferw.p()), mBufferw.length() * 2);
				errors |= !ret ? 8 : 0;
			}
			registry.close();

			// shell open command "module" "%1"
			ret = registry.create(HKEY_CLASSES_ROOT, "Archivarius\\shell\\open\\command");
			errors |= !ret ? 16 : 0;
			if(ret)
			{
				mBufferw.format("\"%s\" \"%%1\"", mBuffer.p());
				ret = registry.setValue("", REG_SZ, reinterpret_cast<byte*>(mBufferw.p()), mBufferw.length() * 2);
				errors |= !ret ? 32 : 0;
			}
			if(errors)
				_printf("registry errors %u\n", errors);
		}

		// system segregation (remove all keys in registry)
		if(FM->mSystemSegregation)
		{
			_printf("system segregation \n");
			uint errors = 0;
			Registry registry;

			// remove .ext
			int ret = registry.remove(HKEY_CLASSES_ROOT, ARCHIVE_EXT);
			errors |= !ret ? 1 : 0;

			ret = registry.remove(HKEY_CLASSES_ROOT, "Archivarius\\DefaultIcon");
			errors |= !ret ? 2 : 0;

			mBuffer = "Archivarius\\shell\\open\\command";
			while(1)
			{
				ret = registry.remove(HKEY_CLASSES_ROOT, mBuffer.p());
				errors |= !ret ? 4 : 0;
				int bs = mBuffer.findr(0x5C);
				if(bs == -1) break;
				mBuffer.resize(bs);
			}

			mBuffer = "Software\\Archivarius\\Options\\Compression";
			forn(3)
			{
				ret = registry.remove(HKEY_CURRENT_USER, mBuffer.p());
				errors |= !ret ? 8 : 0;
				int bs = mBuffer.findr(0x5C);
				if(bs == -1) break;
				mBuffer.resize(bs);
			}
			if(errors)
				_printf("segregation errors %u\n\n", errors);
		}
	}

	return 1;
}

int ArchiverOptions::callback(Widget* widget, uint message, uint_t param)
{
	switch(message)
	{
	case EVM_CLICKED:
	{
		onButtons(param);
		break;
	}
	}
	return 1;
}

void ArchiverOptions::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void ArchiverOptions::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case CMD_SETTING_SAVE:
			saveSettings(0);
			break;
		case FMC_EXIT:
			mWindow->close();
			break;
		}
	}
	mCommands.resize(0);
}

void ArchiverOptions::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		setCommand(CMD_SETTING_SAVE);
		setCommand(FMC_EXIT);
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		setCommand(FMC_EXIT);
	}
}

void ArchiverOptions::onButtons(uint id)
{
	switch(id)
	{
	case ID_OK:
	{
		setCommand(CMD_SETTING_SAVE);
		setCommand(FMC_EXIT);
		break;
	}
	case ID_CANCEL:
	{
		setCommand(FMC_EXIT);
		break;
	}
	}
}

void ArchiverOptions::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

ArchiverOptions* ArchiverOptions::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return ArchiverOptions::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	ArchiverOptions::ptr()->idle();
}

}