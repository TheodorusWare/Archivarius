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
#ifndef _TAH_GuiStringWidget_h_
#define _TAH_GuiStringWidget_h_

#include <Common/Config.h>
#include <Common/Vector2i.h>
#include <Widget/Window.h>
#include <Widget/Label.h>
#include <Widget/Edit.h>
#include <Widget/Checkbox.h>

namespace tas
{

/** String field modal window. */
class StringWidget
{
public:
	/** Interface */
	Window * mWindow;
	Edit   * mStringItem;
	Label  * mNameLabel;

	Vector2i mWindowPos;
	Vector2i mWindowSize;

	Array<half> mCommands; /// commands

	bool mReturn; /// dialog return value, 1 ok, 0 cancel
	String mBuffer; /// string buffer

	static StringWidget* mPtr;

public:
	StringWidget();
	~StringWidget();
	bool create(const String& windowTitle, const String& labelName);
	bool createItems();
	bool initialise();

	bool saveSettings();

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void setCommand(half cmd);

	/** Singleton. */
	static StringWidget* ptr();
};

#define PW StringWidget::ptr()

}

#endif