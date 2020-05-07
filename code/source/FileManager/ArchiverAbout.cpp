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
#include <FileManager/ArchiverAbout.h>
#include <Common/FileAccess.h>
#include <Widget/Button.h>
#include <Widget/Label.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>

namespace tas
{

#define ID_OK  10

ArchiverAbout* ArchiverAbout::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

ArchiverAbout::ArchiverAbout()
{
	mPtr = this;
	mWindow = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(260, 300);
	mReturn = 0;
	mBuffer.reserve(BUFFER_SIZE);
}

ArchiverAbout::~ArchiverAbout()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool ArchiverAbout::create()
{
	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[104], 2, mWindowSize, mWindowPos,
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

bool ArchiverAbout::createItems()
{
	// bitmap
	Vector2i labelPos(mWindowSize.x - 128 >> 1, 10), labelSize(140, 140);
	uint labelId = 11;
	int wstyle = SS_BITMAP;
	Label* label = new Label(mWindow, "", labelId++, wstyle, labelSize, labelPos);

	// information
	wstyle = SS_CENTER;
	labelPos = Vector2i(10, 140);
	labelSize = Vector2i(240, 150);
	mBuffer.format
	(
	    "Archivarius %u.%u.%u\n"
	    "Copyright (C) 2018-2020\n"
	    "Theodorus Software\n"
	    "All rights reserved",
	    TAA_VERSION_MAJOR, TAA_VERSION_MINOR, TAA_VERSION_PATCH
	);
	label = new Label(mWindow, mBuffer, labelId++, wstyle, labelSize, labelPos);

	// button OK
	Vector2i buttonSize(FM->mButtonSize);
	Vector2i buttonPos;
	buttonPos.x = mWindowSize.x - buttonSize.x - 10;
	buttonPos.y = mWindowSize.y - buttonSize.y - 10;
	Button* button = new Button(mWindow, FM->mLocalisation[58], ID_OK, buttonSize, buttonPos);

	return 1;
}

bool ArchiverAbout::initialise()
{
	// button
	Widget* widget = mWindow->findWidgetById(10);
	widget->setFont(FM->mFontSystem);
	// bitmap
	widget = mWindow->findWidgetById(11);
	((Label*)widget)->setImage(FM->mImageLogo);
	// information
	widget = mWindow->findWidgetById(12);
	widget->setFont(FM->mFontLarge);
	return 1;
}

int ArchiverAbout::callback(Widget* widget, uint message, uint_t param)
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

void ArchiverAbout::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void ArchiverAbout::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case FMC_EXIT:
			mWindow->close();
			break;
		}
	}
	mCommands.resize(0);
}

void ArchiverAbout::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_RETURN))
	{
		setCommand(FMC_EXIT);
	}
	else if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		setCommand(FMC_EXIT);
	}
}

void ArchiverAbout::onButtons(uint id)
{
	switch(id)
	{
	case ID_OK:
	{
		setCommand(FMC_EXIT);
		break;
	}
	}
}

void ArchiverAbout::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

ArchiverAbout* ArchiverAbout::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return ArchiverAbout::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	ArchiverAbout::ptr()->idle();
}

}