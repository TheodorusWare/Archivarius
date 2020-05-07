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
#include <FileManager/ProgressWidget.h>
#include <Common/String.h>
#include <Common/StringLib.h>
#include <Common/ConvertBytes.h>
#include <Widget/Event.h>
#include <Widget/Keyboard.h>
#include <Widget/Button.h>

namespace tas
{

#define ID_CANCEL   10

ProgressWidget* ProgressWidget::mPtr = 0;

static int  WidgetCallbackImpl(Widget* widget, uint message, uint_t param);
static void IdleCallbackImpl();

ProgressWidget::ProgressWidget()
{
	mPtr = this;
	mWindowPos = Vector2i(10, 10);
	mWindowSize = Vector2i(460, 270);
	mReturn = 1;
	mType = 0;
	mClose = 0;
	mWindow = 0;
	mProgressbar = 0;
	mFileName.reserve(BUFFER_SIZE);
	mBuffer.reserve(BUFFER_SIZE);
	forn(8) mLabels[i] = 0;
}

ProgressWidget::~ProgressWidget()
{
	safe_delete(mWindow);
	mPtr = 0;
}

bool ProgressWidget::create(byte type)
{
	mType = type;
	FM->mWindow->enable(0);

	// Archiving Unarchiving Testing
	mWindow = new Window(FM->mWindow, FM->mLocalisation[89 + mType], 3, mWindowSize, mWindowPos,
	                     WidgetCallbackImpl, IdleCallbackImpl);

	uint windowProps = 12;

	// create widgets
	createItems();

	// create main window
	mWindow->createEx(windowProps, 1);

	// initialise widgets
	initialise();

	// main loop
	// mWindow->run();

	// FM->mWindow->enable(1);

	return mReturn;
}

bool ProgressWidget::createItems()
{
	Vector2i labelPos(10, 20), labelSize(mWindowSize.x - 20, 25);
	uint labelId = 11;
	int wstyle = SS_CENTER;
	/* char* labelNames[] =
	{
		"Time", "Speed", "Ratio", "Processed files",
		"Processed size", "Total size"
	}; */
	uint labelLocId[] = { 22, 92, 72, 93, 94, 95 };
	uint descWidth = 120;
	uint valueWidth = 90;
	uint partWidth2 = descWidth + valueWidth + 10;
	uint partWidth1 = mWindowSize.x - partWidth2;

	// labels
	forn(14)
	{
		wchar_t* caption = L"";
		if(i >= 2 and i <= 7)
			caption = FM->mLocalisation[labelLocId[i - 2]].p();

		Label* label = new Label(mWindow, caption, labelId++, wstyle, labelSize, labelPos);

		if(i < 2)
			mLabels[i] = label;
		if(i >= 8)
			mLabels[i-6] = label;

		if(!i) // file name
		{
			labelPos.y = 80;
			wstyle = SS_LEFT | SS_ENDELLIPSIS;
		}
		else
		{
			byte resetPosY = 0;
			if(i == 1 or i == 4) // descriptions
			{
				wstyle = SS_LEFT;
				labelPos.y = 120;
				if(i == 1)
					labelSize.x = descWidth;
				if(i == 4)
				{
					labelPos.x = partWidth1;
				}
				resetPosY = 1;
			}
			if(i == 7 or i == 10) // values
			{
				wstyle = SS_RIGHT;
				labelPos.y = 120;
				if(i == 7)
				{
					labelPos.x = descWidth;
					labelSize.x = valueWidth;
				}
				else
				{
					labelPos.x = partWidth1 + descWidth;
				}
				resetPosY = 1;
			}
			if(!resetPosY)
				labelPos.y += 25;
		}
	}

	// progress bar
	Vector2i progSize(mWindowSize.x - 20, 24);
	Vector2i progPos(10, 50);
	mProgressbar = new Progressbar(mWindow, "", labelId++, progSize, progPos);

	// button cancel
	Vector2i buttonSize(FM->mButtonSize);
	Vector2i buttonPos(mWindowSize.x - buttonSize.x - 10, mWindowSize.y - buttonSize.y - 10);
	Button* button = new Button(mWindow, FM->mLocalisation[59], ID_CANCEL, buttonSize, buttonPos);

	return 1;
}

bool ProgressWidget::initialise()
{
	forn(15)
	{
		Widget* label = mWindow->findWidgetById(10 + i);
		assert(label);
		if(!i)
			label->setFont(FM->mFontSystem);
		else
			label->setFont(FM->mFontLarge);
	}
	mProgressbar->setRange(100);
	mProgressbar->setPositionBar(0);
	return 1;
}

bool ProgressWidget::operationCancel()
{
	// Stop?
	int ret = MessageBox(mWindow->getHandle(), FM->mLocalisation[96].p(), FM->mLocalisation[89 + mType].p(), MB_OKCANCEL | MB_ICONQUESTION);
	FM->mKeyboard.setKeyState(VK_RETURN, 1);
	FM->mKeyboard.setKeyState(VK_ESCAPE, 1);
	if(ret == IDOK)
	{
		mClose = 1;
		mWindow->close();
		mReturn = 0;
		return 1;
	}
	return 0;
}

bool ProgressWidget::closeWindow()
{
	if(!mWindow->isActivated() and mType != 2)
		FM->setCommand(FMC_DEACTIVATE);
	mClose = 1;
	mWindow->close();
	SystemMessage();
	return 1;
}

int ProgressWidget::callback(Widget* widget, uint message, uint_t param)
{
	switch(message)
	{
	case EVM_CLICKED:
	{
		onButtons(param);
		break;
	}
	case EVM_CLOSE:
	{
		if(!mClose)
		{
			setCommand(ID_CANCEL);
			return 0;
		}
		break;
	}
	}
	return 1;
}

void ProgressWidget::idle()
{
	if(!mWindow->isActivated()) return;
	commands();
	keyboard();
}

void ProgressWidget::commands()
{
	forn(mCommands.size())
	{
		switch(mCommands[i])
		{
		case ID_CANCEL:
			operationCancel();
			break;
		}
	}
	mCommands.resize(0);
}

void ProgressWidget::keyboard()
{
	if(FM->mKeyboard.getKeyPressed(VK_ESCAPE))
	{
		setCommand(ID_CANCEL);
	}
}

void ProgressWidget::onButtons(uint id)
{
	switch(id)
	{
	case ID_CANCEL:
	{
		setCommand(ID_CANCEL);
		break;
	}
	}
}

void ProgressWidget::setCommand(half cmd)
{
	mCommands.push_back(cmd);
}

int ProgressWidget::eventCallback(wint* data)
{
	if(!SystemMessage() or !mReturn)
		return 0;
	idle();

	// skip event callback
	if(!data) return 1;

	// progress position
	assert(data[1]);
	uint progress = (u64)data[0] * 100 / data[1];
	mProgressbar->setPositionBar(progress);

	// file name pointer
	if(data[5] != 0)
	{
		String* fileName = (String*)data[5];
		uint bs = fileName->findr(0x5C) + 1;
		uint len = fileName->length() - bs;
		if(len > 100) len = 100;
		mFileName.assign(fileName->p() + bs, len);
	}

	if(mTimer.getMilliseconds() < 200) return 1;
	mTimer.reset();

	// file name
	if(mFileName.length() != 0)
	{
		mLabels[1]->setText(mFileName);
		mFileName.clear();
	}

	// progress text
	mBuffer.format("%u %%", progress);
	mLabels[0]->setText(mBuffer);

	// time [3]
	uint* time = (uint*) data[6];
	mBuffer.format("%02u:%02u:%03u", time[0], time[1], time[2] - time[2] % 100);
	mLabels[2]->setText(mBuffer);

	// speed / sec
	wint timeSec = time[0] * 60000 + time[1] * 1000 + time[2];
	wint speed = (u64)data[0] * 1000 / timeSec;
	uint rem = 0;
	uint div = convertBytesToDecimal(speed, rem);
	mBuffer.format("%u.%u %ls/%ls", static_cast<uint>(speed), rem / 104,
	               FM->mLocalisation[133 + div].p(), FM->mLocalisation[137].p());
	mLabels[3]->setText(mBuffer);

	// ratio
	if(data[2])
	{
		mBuffer.format("%u %%", (uint)data[2]);
		mLabels[4]->setText(mBuffer);
	}

	// processed files
	mBuffer.format("%u / %u", (uint)data[3], (uint)data[4]);
	mLabels[5]->setText(mBuffer);

	// processed, total size
	forn(2)
	{
		wint prcSize = data[i];
		div = convertBytesToDecimal(prcSize, rem);
		mBuffer.format("%u.%u %ls", static_cast<uint>(prcSize), rem / 104, FM->mLocalisation[133 + div].p());
		mLabels[6 + i]->setText(mBuffer);
	}

	return 1;
}

ProgressWidget* ProgressWidget::ptr()
{
	return mPtr;
}

int WidgetCallbackImpl(Widget* widget, uint message, uint_t param)
{
	return ProgressWidget::ptr()->callback(widget, message, param);
}

void IdleCallbackImpl()
{
	ProgressWidget::ptr()->idle();
}

int EventCallbackImpl(wint* data)
{
	return ProgressWidget::ptr()->eventCallback(data);
}

}