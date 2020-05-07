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
#ifndef _TAH_GuiArchiverOptions_h_
#define _TAH_GuiArchiverOptions_h_

#include <Common/Config.h>
#include <Common/Vector2i.h>

#include <Widget/Window.h>
#include <Widget/Checkbox.h>

namespace tas
{

/** Archiver options modal window. */
class ArchiverOptions
{
public:
	/** Interface */
	Window   * mWindow;
	Checkbox * mOptionItems[5];
	Combobox * mSettings[2];

	Vector2i mWindowPos;
	Vector2i mWindowSize;

	Array<half> mCommands; /// commands

	bool mReturn; /// dialog return value, 1 ok, 0 cancel
	String mBuffer; /// string buffer
	String mBufferw;
	byte mInitState[3]; /// state first 3 checkbox
	byte mCurState[4]; /// current state first 4 checkbox

	static ArchiverOptions* mPtr;

public:
	ArchiverOptions();
	~ArchiverOptions();
	bool create();
	bool createItems();
	bool initialise();
	bool saveSettings(byte mode);

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void setCommand(half cmd);

	/** Singleton. */
	static ArchiverOptions* ptr();
};

}

#endif