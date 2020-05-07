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
#include <FileManager/PasswordWidget.h>
#include <Common/FileAccess.h>
#include <Common/String.h>
#include <Common/StringLib.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>

namespace tas
{

#define WINDOW_TITLE  "Archive password"

#define ID_OK        10
#define ID_CANCEL    11
#define ID_PASSWORD  12

#define ENC_SETTING_SAVE    10
#define ENC_PASSWORD        11

PasswordWidget* PasswordWidget::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

PasswordWidget::PasswordWidget()
{
	mPtr = this;
	mWindow = 0;
	mPasswordItem = 0;
	mPasswordShowItem = 0;
	mArchiveNameLabel = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(320, 180);
	mReturn = 0;
	mBuffer.reserve(BUFFER_SIZE);
}

PasswordWidget::~PasswordWidget()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool PasswordWidget::create(const String& archiveName)
{
	mBuffer = archiveName;

	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[44], 2, mWindowSize, mWindowPos,
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

bool PasswordWidget::createItems()
{
	Vector2i labelPos(10, 20), labelSize(50, 30),
	         editPos (10, 42), editSize (300, 20),
	         cbPos   (10, 72), cbSize   (120, 20);

	uint editId  = 13;
	uint labelId = 14;
	int wstyle = 0;

	// labels 2 password, archive name
	forn(2)
	{
		Label* label = new Label(mWindow, FM->mLocalisation[44], labelId++, wstyle, labelSize, labelPos);
		labelPos.x = 70;
		wstyle = SS_ENDELLIPSIS | SS_RIGHT;
		labelSize.x = mWindowSize.x - labelPos.x - 10;
		if(i) mArchiveNameLabel = label;
	}

	bool pass = 0;
	if(FM->mArchiverSettings.showPass != 0xFF)
		pass = !FM->mArchiverSettings.showPass;

	// password field
	editSize.x = mWindowSize.x - 20;
	mPasswordItem = new Edit(mWindow, "", editId, pass, editSize, editPos);

	// password show
	mPasswordShowItem = new Checkbox(mWindow, FM->mLocalisation[106], ID_PASSWORD, cbSize, cbPos);

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

bool PasswordWidget::initialise()
{
	forn(6)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		label->setFont(FM->mFontSystem);
	}
	if(FM->mArchiverSettings.showPass != 0xFF)
		mPasswordShowItem->setState(FM->mArchiverSettings.showPass);
	mArchiveNameLabel->setText(mBuffer);
	mPasswordItem->setFocus();
	FM->mPassword.clear();
	return 1;
}

bool PasswordWidget::saveSettings()
{
	mReturn = 1;

	mPasswordItem->getText(FM->mPassword);
	FM->mPassword.update();

	if(!FM->mPassword.length())
		return mReturn = 0;

	FM->mArchiverSettings.showPass = mPasswordShowItem->getState();

	return 1;
}

bool PasswordWidget::switchPassword()
{
	mPasswordItem->recreate(!mPasswordShowItem->getState());
	mPasswordItem->setFont(FM->mFontSystem);
	return 1;
}

int PasswordWidget::callback(Widget* widget, uint message, uint_t param)
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

void PasswordWidget::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void PasswordWidget::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case FMC_EXIT:
			mWindow->close();
			break;
		case ENC_SETTING_SAVE:
			saveSettings();
			break;
		case ENC_PASSWORD:
			switchPassword();
			break;
		}
	}
	mCommands.resize(0);
}

void PasswordWidget::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		// save state
		setCommand(ENC_SETTING_SAVE);
		setCommand(FMC_EXIT);
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		setCommand(FMC_EXIT);
	}
}

void PasswordWidget::onButtons(uint id)
{
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
	case ID_PASSWORD:
	{
		setCommand(ENC_PASSWORD);
		break;
	}
	}
}

void PasswordWidget::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

PasswordWidget* PasswordWidget::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return PasswordWidget::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	PasswordWidget::ptr()->idle();
}

}