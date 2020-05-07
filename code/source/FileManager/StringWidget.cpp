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
#include <FileManager/StringWidget.h>
#include <Common/StringLib.h>
#include <Common/String.h>
#include <Common/FileAccess.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>

namespace tas
{

#define ID_OK        10
#define ID_CANCEL    11

StringWidget* StringWidget::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

StringWidget::StringWidget()
{
	mPtr = this;
	mWindow = 0;
	mStringItem = 0;
	mNameLabel = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(320, 130);
	mReturn = 0;
	mBuffer.reserve(BUFFER_SIZE);
}

StringWidget::~StringWidget()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool StringWidget::create(const String& windowTitle, const String& labelName)
{
	mBuffer = labelName;

	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, windowTitle, 2, mWindowSize, mWindowPos,
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

bool StringWidget::createItems()
{
	Vector2i labelPos(10, 10), labelSize(50, 30),
	         editPos (10, 32), editSize (300, 20);

	uint editId  = 12;
	uint labelId = 13;
	int wstyle = 0;

	// label
	mNameLabel = new Label(mWindow, mBuffer, labelId++, wstyle, labelSize, labelPos);

	// string field
	editSize.x = mWindowSize.x - 20;
	mStringItem = new Edit(mWindow, "", editId, 0, editSize, editPos);

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

bool StringWidget::initialise()
{
	forn(4)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		label->setFont(FM->mFontSystem);
	}
	mStringItem->setFocus();
	mBuffer.clear();
	return 1;
}

bool StringWidget::saveSettings()
{
	mReturn = 1;

	mStringItem->getText(mBuffer);
	mBuffer.update();

	if(!mBuffer.length())
		return mReturn = 0;

	return 1;
}

int StringWidget::callback(Widget* widget, uint message, uint_t param)
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

void StringWidget::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void StringWidget::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case ID_OK:
			saveSettings();
			break;
		case FMC_EXIT:
			mWindow->close();
			break;
		}
	}
	mCommands.resize(0);
}

void StringWidget::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		// save state
		setCommand(ID_OK);
		setCommand(FMC_EXIT);
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		setCommand(FMC_EXIT);
	}
}

void StringWidget::onButtons(uint id)
{
	_printf("onButtons %u \n", id);
	switch(id)
	{
	case ID_OK:
	{
		setCommand(ID_OK);
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

void StringWidget::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

StringWidget* StringWidget::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return StringWidget::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	StringWidget::ptr()->idle();
}

}