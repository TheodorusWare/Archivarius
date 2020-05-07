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
#ifndef _TAH_GuiProgressWidget_h_
#define _TAH_GuiProgressWidget_h_

#include <Common/Config.h>
#include <Common/Vector2i.h>
#include <Common/Timer.h>
#include <Widget/Window.h>
#include <Widget/Label.h>
#include <Widget/Progressbar.h>

namespace tas
{

class ProgressWidget
{
public:
	/** Interface */
	Window      * mWindow;
	Label       * mLabels[8];
	Progressbar * mProgressbar;

	Timer mTimer;

	Vector2i mWindowPos;
	Vector2i mWindowSize;

	Array<half> mCommands; /// commands

	String mFileName;
	String mBuffer; /// string buffer

	bool mReturn; /// dialog return value, 1 ok, 0 cancel
	bool mClose;
	byte mType;   /// encode 0, decode 1, test 2

	static ProgressWidget* mPtr;

public:
	ProgressWidget();
	~ProgressWidget();

	/** Return nil after pressed cancel
		 Type encode 0, decode 1 */
	bool create(byte type);
	bool createItems();
	bool initialise();
	bool operationCancel();
	bool closeWindow();

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void setCommand(half cmd);

	/** Archive callback. */
	int eventCallback(wint* data);

	/** Singleton. */
	static ProgressWidget* ptr();
};

}

#endif