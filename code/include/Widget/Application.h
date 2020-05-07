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
#ifndef _TAH_GuiApplication_h_
#define _TAH_GuiApplication_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/container/Array.h>
#include <Common/Vector2i.h>
#include <Widget/Event.h>

namespace tas
{

class Window;

class Message
{
public:
	uint message;
	uint_t param;
	Message();
	Message(uint message, uint_t param);
	~Message();
};

/** Base class for Windows, Buttons .. */
class GuiApplication
{
	static GuiApplication* mPtr;
	Array<Window*> mWindows;
	Array<Message> mMessages; /// messages from window procedure
public:
	GuiApplication();
	~GuiApplication();

	void registerWindow(Window* window);
	void unregisterWindow(Window* window);

	Window* getLastWindow();
	Window* getWindow(uint i);
	Window* findWindowByHandle(HWND handle);

	/** main loop */
	// bool run();

	/** idle */
	bool onIdle();

	/** from window procedure */
	bool setMessage(uint message, uint_t param);

	/** return HFONT */
	void* createFont(const String& name, int height, int weight = 400);

	/** Singleton. */
	static GuiApplication* ptr();
};

#define WT_APP GuiApplication::ptr()

}

#endif