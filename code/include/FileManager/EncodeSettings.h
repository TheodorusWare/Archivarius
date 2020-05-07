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
#ifndef _TAH_GuiEncodeSettings_h_
#define _TAH_GuiEncodeSettings_h_

#include <Common/Config.h>
#include <Common/Vector2i.h>

#include <Widget/Window.h>
#include <Widget/Checkbox.h>
#include <Widget/Combobox.h>
#include <Widget/Edit.h>

namespace tas
{

/// Archive params
struct EncodeParams
{
	uint level;
	uint method;
	uint dictSize;
	uint matchMin;
	uint matchMax;
	uint threads;
	uint bigraph;
	uint crypt;
	uint suffix;
	uint matchCycles;
	uint rolz;
	uint cmix;
	EncodeParams();
	~EncodeParams();
	void reset();
	uint& operator[](int i);
};

/** Compression settings modal window. */
class EncodeSettings
{
public:
	/** Interface */
	Window   * mWindow;
	Edit     * mArchiveName;
	Edit     * mParameters;
	Edit     * mPassword;
	Checkbox * mPreproc;
	Checkbox * mCrypt;
	Checkbox * mPasswordShow;
	Combobox * mSettings[6];

	Vector2i mWindowPos;
	Vector2i mWindowSize;

	/** Archive params */
	EncodeParams mEncodeParams;

	Array<half> mCommands; /// commands

	byte mCompressionLevel;
	byte mCompressionMethod;

	bool mReturn; /// dialog return value, 1 ok, 0 cancel

	uint mSelectId;
	bool mDropdown;

	String mBuffer; /// string buffer

	String mArchiveDirectory;
	String mArchiveNames;

	static EncodeSettings* mPtr;

public:
	EncodeSettings();
	~EncodeSettings();
	// bool create(char* arcDir, char* arcName);
	bool create(const String& arcDir, const String& arcName);
	bool createItems();
	bool initialise();

	bool initialiseSettings(byte level);
	bool updateSettings();
	bool saveSettings();
	bool fileDialog();
	bool switchPassword();
	bool parsingParameters();

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void setCommand(half cmd);

	/** Singleton. */
	static EncodeSettings* ptr();
};

#define ES EncodeSettings::ptr()

}

#endif