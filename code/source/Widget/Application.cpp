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
#include <Common/platform/swindows.h>
#include <Common/StringLib.h>
#include <Widget/Application.h>
#include <Widget/Window.h>
#include <Widget/Event.h>
#include <Widget/Combobox.h>

namespace tas
{

Message::Message()
{
	message = 0;
	param = 0;
}

Message::Message(uint _message, uint_t _param)
{
	message = _message;
	param = _param;
}

Message::~Message()
{
}

GuiApplication* GuiApplication::mPtr = 0;

GuiApplication::GuiApplication()
{
	mPtr = this;
}

GuiApplication::~GuiApplication()
{
	mWindows.clear_ptr();
	mPtr = 0;
}

void GuiApplication::registerWindow(Window* window)
{
	mWindows.push_back(window);
}

void GuiApplication::unregisterWindow(Window* window)
{
	int n = mWindows.find(window);
	if(n != -1)
	{
		_printf("GuiApplication::unregisterWindow %u\n", window->getId());
		mWindows.erase(n);
	}
}

Window* GuiApplication::getLastWindow()
{
	if(!mWindows.size()) return 0;
	return mWindows[mWindows.size()-1];
}

Window* GuiApplication::getWindow(uint i)
{
	assert(i < mWindows.size());
	return mWindows[i];
}

Window* GuiApplication::findWindowByHandle(HWND handle)
{
	forn(mWindows.size())
	{
		if(mWindows[i]->getHandle() == handle)
			return mWindows[i];
	}
	return 0;
}

bool GuiApplication::onIdle()
{
	/* process all messages from queue */
	forn(mMessages.size())
	{
		Message* cm = &mMessages[i];
		// combobox end edit, kill focus widget
		if(cm->message == 1)
		{
			Combobox* cb = (Combobox*) cm->param;
			cb->onEvent(2, 0);
		}
		if(cm->message == 2)
		{
			Combobox* cb = (Combobox*) cm->param;
			cb->onEvent(3, 0);
		}
	}
	mMessages.resize(0);
	return 1;
}

bool GuiApplication::setMessage(uint message, uint_t param)
{
	mMessages.push_back(Message(message, param));
	return 1;
}

void* GuiApplication::createFont(const String& name, int height, int weight)
{
	if(!mWindows.size()) return 0;
	int fsize = -MulDiv(height, GetDeviceCaps(GetDC(mWindows[0]->getHandle()), LOGPIXELSY), 72);
	void* fontHandle = CreateFont(fsize, 0, 0, 0, weight, false, false, false, DEFAULT_CHARSET,
	                              OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, name.p());
	return fontHandle;
}

GuiApplication* GuiApplication::ptr()
{
	return mPtr;
}

}