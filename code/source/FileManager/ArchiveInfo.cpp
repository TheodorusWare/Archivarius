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
#include <FileManager/ArchiveInfo.h>
#include <Common/String.h>
#include <Common/StringLib.h>
#include <Common/FileAccess.h>
#include <Common/ConvertBytes.h>
#include <Widget/Button.h>
#include <Widget/Label.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>

namespace tas
{

#define ID_OK  10

ArchiveInfo* ArchiveInfo::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

ArchiveInfo::ArchiveInfo()
{
	mPtr = this;
	mWindow = 0;
	mgInfoList = 0;
	mReturn = 0;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(340, 380);
	mBuffer.reserve(BUFFER_SIZE);
}

ArchiveInfo::~ArchiveInfo()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool ArchiveInfo::create()
{
	FM->mWindow->enable(0);

	mWindow = new Window(FM->mWindow, FM->mLocalisation[67], 2, mWindowSize, mWindowPos,
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

bool ArchiveInfo::createItems()
{
	// information list
	uint listHeight = mWindowSize.y - FM->mButtonSize.y - 20;
	Vector2i mInfoListPos = Vector2i(0, 0);
	Vector2i mInfoListSize = Vector2i(mWindowSize.x, listHeight);
	mgInfoList = new Listview(mWindow, ID_WC_FILELIST, mInfoListSize, mInfoListPos);

	// button OK
	Vector2i buttonSize(FM->mButtonSize);
	Vector2i buttonPos;
	buttonPos.x = mWindowSize.x - buttonSize.x - 10;
	buttonPos.y = mWindowSize.y - buttonSize.y - 10;
	Button* button = new Button(mWindow, FM->mLocalisation[58], ID_OK, buttonSize, buttonPos);

	return 1;
}

bool ArchiveInfo::initialise()
{
	// Name Value
	mgInfoList->setColumn(FM->mLocalisation[20], 0, 140, 0, 0);
	mgInfoList->setColumn(FM->mLocalisation[68], 1, 200, 0, 0);
	mgInfoList->setExtendedStyle(1);

	// archive name 0
	mBuffer.assign(FM->mArchive.getName()).substrxs(0x5C, 1, 1);
	mgInfoList->insertItem(0);
	mgInfoList->setItem(FM->mLocalisation[36], 0, 0, 0);
	mgInfoList->setItem(mBuffer, 0, 1, 0);

	// files count 1
	mBuffer.format("%u", FM->mArchive.getValue(8));
	mgInfoList->insertItem(1);
	mgInfoList->setItem(FM->mLocalisation[69], 1, 0, 0);
	mgInfoList->setItem(mBuffer, 1, 1, 0);

	// source size 2
	wint valueBig = FM->mArchive.getValue(7);
	convertDecimalSpace(&mBuffer, valueBig);
	mgInfoList->insertItem(2);
	mgInfoList->setItem(FM->mLocalisation[70], 2, 0, 0);
	mgInfoList->setItem(mBuffer, 2, 1, 0);

	// compress size 3
	valueBig = FM->mArchive.getValue(9);
	convertDecimalSpace(&mBuffer, valueBig);
	mgInfoList->insertItem(3);
	mgInfoList->setItem(FM->mLocalisation[71], 3, 0, 0);
	mgInfoList->setItem(mBuffer, 3, 1, 0);

	// ratio 4
	uint value = FM->mArchive.getValue(4);
	mBuffer.format("%u / 100", value);
	mgInfoList->insertItem(4);
	mgInfoList->setItem(FM->mLocalisation[72], 4, 0, 0);
	mgInfoList->setItem(mBuffer, 4, 1, 0);

	uint i = 5;

#define SPLIT_LINE ("____________________")
	mgInfoList->insertItem(i);
	mgInfoList->setItem(SPLIT_LINE, i, 0, 0);
	mgInfoList->setItem(SPLIT_LINE, i++, 1, 0);

	// method
	value = FM->mArchive.getValue(2);
	uint method = value;

	switch(value)
	{
	case 0:
		mBuffer = FM->mLocalisation[88]; // None
		break;
	case 1:
		mBuffer = "LZRE";
		break;
	case 2:
		mBuffer = "CM";
		break;
	}
	mgInfoList->insertItem(i);
	// mgInfoList->setItem("Method", i, 0, 0);
	mgInfoList->setItem(FM->mLocalisation[73], i, 0, 0);
	mgInfoList->setItem(mBuffer, i++, 1, 0);

	uint level;

	if(method != 0)
	{
		// level
		level = FM->mArchive.getValue(3);
		mBuffer.format("%u", level);
		mgInfoList->insertItem(i);
		// mgInfoList->setItem("Level", i, 0, 0);
		mgInfoList->setItem(FM->mLocalisation[74], i, 0, 0);
		mgInfoList->setItem(mBuffer, i++, 1, 0);

		// dictionary
		wint walue = FM->mArchive.getValue(11);
		uint rem = 0;
		uint divn = convertBytesToDecimal(walue, rem);
		mBuffer.format("%u %ls", static_cast<uint>(walue), FM->mLocalisation[133 + divn].p());
		mgInfoList->insertItem(i);
		// mgInfoList->setItem("Dictionary", i, 0, 0);
		mgInfoList->setItem(FM->mLocalisation[75], i, 0, 0);
		mgInfoList->setItem(mBuffer, i++, 1, 0);
	}
	// lzre
	if(method == 1)
	{
		// match
		uint min = FM->mArchive.getValue(12);
		uint max = FM->mArchive.getValue(13);
		mBuffer.format("[%u, %u]", min, max);
		mgInfoList->insertItem(i);
		// mgInfoList->setItem("Match", i, 0, 0);
		mgInfoList->setItem(FM->mLocalisation[76], i, 0, 0);
		mgInfoList->setItem(mBuffer, i++, 1, 0);

		// rolz
		value = FM->mArchive.getValue(14);
		if(level > 6)
		{
			if(value)
				value = 64 << value - 1;
			else
				value = 16;
			mBuffer.format("%u", value);
			mgInfoList->insertItem(i);
			// mgInfoList->setItem("Rolz", i, 0, 0);
			mgInfoList->setItem(FM->mLocalisation[77], i, 0, 0);
			mgInfoList->setItem(mBuffer, i++, 1, 0);
		}

		// threads
		value = FM->mArchive.getValue(15);
		mBuffer.format("%u", value);
		mgInfoList->insertItem(i);
		// mgInfoList->setItem("Threads", i, 0, 0);
		mgInfoList->setItem(FM->mLocalisation[78], i, 0, 0);
		mgInfoList->setItem(mBuffer, i++, 1, 0);

		// context mixing
		value = FM->mArchive.getValue(16);
		if(value)
		{
			mgInfoList->insertItem(i);
			// mgInfoList->setItem("Context mixing", i, 0, 0);
			mgInfoList->setItem(FM->mLocalisation[79], i, 0, 0);
			mgInfoList->setItem(FM->mLocalisation[87], i++, 1, 0); // enable
		}
	}
	// context mixing
	if(method == 2)
	{
		// context length
		uint max = FM->mArchive.getValue(13);
		mBuffer.format("%u", max);
		mgInfoList->insertItem(i);
		// mgInfoList->setItem("Context", i, 0, 0);
		mgInfoList->setItem(FM->mLocalisation[80], i, 0, 0);
		mgInfoList->setItem(mBuffer, i++, 1, 0);
	}

	mgInfoList->insertItem(i);
	mgInfoList->setItem(SPLIT_LINE, i, 0, 0);
	mgInfoList->setItem(SPLIT_LINE, i++, 1, 0);

	// crypt
	value = FM->mArchive.getValue(5);
	switch(value)
	{
	case 0:
		mBuffer = FM->mLocalisation[88]; // "None";
		break;
	case 1:
		mBuffer = FM->mLocalisation[82]; // "Names";
		break;
	case 2:
		mBuffer = FM->mLocalisation[83]; // "Data";
		break;
	case 3:
		mBuffer = FM->mLocalisation[84]; // "All";
		break;
	}
	mgInfoList->insertItem(i);
	// mgInfoList->setItem("Encryption", i, 0, 0);
	mgInfoList->setItem(FM->mLocalisation[81], i, 0, 0);
	mgInfoList->setItem(mBuffer, i++, 1, 0);

	// preprocessing
	value = FM->mArchive.getValue(6);
	switch(value)
	{
	case 0:
		mBuffer = FM->mLocalisation[88]; // "None";
		break;
	case 1:
		mBuffer = FM->mLocalisation[86]; // "Bigraph";
		break;
	}
	mgInfoList->insertItem(i);
	// mgInfoList->setItem("Preprocess", i, 0, 0);
	mgInfoList->setItem(FM->mLocalisation[85], i, 0, 0);
	mgInfoList->setItem(mBuffer, i++, 1, 0);

	// set button system font
	Widget* button = mWindow->findWidgetById(ID_OK);
	button->setFont(FM->mFontSystem);
	mgInfoList->setFocus();
	return 1;
}

int ArchiveInfo::callback(Widget* widget, uint message, uint_t param)
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

void ArchiveInfo::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void ArchiveInfo::commands()
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

void ArchiveInfo::keyboard()
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

void ArchiveInfo::onButtons(uint id)
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

void ArchiveInfo::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

ArchiveInfo* ArchiveInfo::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return ArchiveInfo::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	ArchiveInfo::ptr()->idle();
}

}