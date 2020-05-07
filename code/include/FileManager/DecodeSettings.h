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
#ifndef _TAH_GuiDecodeSettings_h_
#define _TAH_GuiDecodeSettings_h_

#include <Common/Config.h>
#include <Common/Vector2i.h>
#include <Widget/Window.h>
#include <Widget/Edit.h>
#include <Widget/Combobox.h>

namespace tas
{

/** Uncompression settings modal window. */
class DecodeSettings
{
public:
	/** Interface */
	Window   * mWindow;
	Edit     * mDirectoryItem;
	Combobox * mPathsItem;

	Vector2i mWindowPos;
	Vector2i mWindowSize;

	/** Archive params */
	String mDirectory;

	Array<half> mCommands; /// commands

	bool mReturn; /// dialog return value, 1 ok, 0 cancel
	bool mDropdown;

	String mBuffer; /// string buffer

	static DecodeSettings* mPtr;

public:
	DecodeSettings();
	~DecodeSettings();
	bool create(const String& directory);
	bool createItems();
	bool initialise();

	bool saveSettings();
	bool fileDialog();

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void setCommand(half cmd);

	/** Singleton. */
	static DecodeSettings* ptr();
};

#define DS DecodeSettings::ptr()

}

#endif