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
#include <FileManager/DecodeSettings.h>
#include <Common/StringLib.h>
#include <Common/FileAccess.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>
#include <Widget/Checkbox.h>
#include <Widget/Label.h>

namespace tas
{

#define ID_FILE_DLG  10
#define ID_OK        11
#define ID_CANCEL    12

#define DEC_FILE_DIALOG   10
#define DEC_SETTING_SAVE  11

DecodeSettings* DecodeSettings::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

DecodeSettings::DecodeSettings()
{
	mPtr = this;
	mWindow = 0;
	mPathsItem = 0;
	mDropdown = 0;
	mReturn = 0;
	mDirectoryItem = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(340, 200);
	mBuffer.reserve(BUFFER_SIZE);
	mDirectory.reserve(BUFFER_SIZE);
}

DecodeSettings::~DecodeSettings()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool DecodeSettings::create(const String& directory)
{
	mDirectory = directory;

	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[60], 2, mWindowSize, mWindowPos,
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

bool DecodeSettings::createItems()
{
	Vector2i labelPos(10, 20), labelSize(200, 20),
	         editPos (10, 42), editSize (300, 20),
	         comboPos(10, 94), comboSize(120, 20);

	uint editId  = 13;
	uint comboId = 14;
	uint labelId = 15;

	// labels 2 Output directory, File paths
	forn(2)
	{
		Label* label = new Label(mWindow, FM->mLocalisation[61 + i], labelId++, 0, labelSize, labelPos);
		labelPos.y += 52;
	}

	// output directory
	editSize.x = mWindowSize.x - 20;
	mDirectoryItem = new Edit(mWindow, "", editId, 0, editSize, editPos);

	// file paths
	mPathsItem = new Combobox(mWindow, "", comboId, 0, comboSize, comboPos);

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

	return 1;
}

bool DecodeSettings::initialise()
{
	forn(7)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		label->setFont(FM->mFontSystem);
	}
	Widget* item = mWindow->findWidgetById(ID_FILE_DLG);
	item->createTooltip(FM->mLocalisation[51]); // File dialog
	mDirectoryItem->setText(mDirectory);

	// Without paths, Relative paths, Full paths
	forn(3)
	mPathsItem->appendString(FM->mLocalisation[63 + i]);
	mPathsItem->setCurrentItem(FM->mArchiverSettings.filePath);
	return 1;
}

bool DecodeSettings::saveSettings()
{
	mReturn = 1;
	mDirectoryItem->getText(mBuffer);
	mBuffer.update();
	if(mBuffer.length())
		mDirectory = mBuffer;
	FM->mArchiverSettings.filePath = mPathsItem->getCurrentItem();
	return 1;
}

bool DecodeSettings::fileDialog()
{
	// All files (*.*)\0*.*\0\0
	String maskFile(FM->mLocalisation[56]);
	maskFile.append(L" (*.*)\0*.*\0\0", 15);
	String initialDir = mDirectory.substrx(0x5C, 1, 0);
	mBuffer = mDirectory.substrx(0x5C, 1, 1);
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
	ofn.lpstrInitialDir = initialDir.p();
	ofn.Flags = OFN_EXPLORER | OFN_NOVALIDATE;
	ofn.lpstrTitle = FM->mLocalisation[66].p(); // Select output directory
	if(GetOpenFileName(&ofn))
	{
		mDirectory = ofn.lpstrFile;
		mDirectoryItem->setText(mDirectory);
	}
	FM->mKeyboard.setKeyState(VK_ESCAPE, 1);
	FM->mKeyboard.setKeyState(VK_RETURN, 1);
	return 1;
}

int DecodeSettings::callback(Widget* widget, uint message, uint_t param)
{
	switch(message)
	{
	case EVM_CLICKED:
	{
		onButtons(param);
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

void DecodeSettings::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void DecodeSettings::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case FMC_EXIT:
			mWindow->close();
			break;
		case DEC_SETTING_SAVE:
			saveSettings();
			break;
		case DEC_FILE_DIALOG:
			fileDialog();
			break;
		}
	}
	mCommands.resize(0);
}

void DecodeSettings::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		if(!mDropdown)
		{
			setCommand(DEC_SETTING_SAVE);
			setCommand(FMC_EXIT);
		}
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		if(!mDropdown)
			setCommand(FMC_EXIT);
	}
}

void DecodeSettings::onButtons(uint id)
{
	switch(id)
	{
	case ID_OK:
	{
		setCommand(DEC_SETTING_SAVE);
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
		setCommand(DEC_FILE_DIALOG);
		break;
	}
	}
}

void DecodeSettings::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

DecodeSettings* DecodeSettings::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return DecodeSettings::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	DecodeSettings::ptr()->idle();
}

}