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
#ifndef _TAH_GuiWidget_h_
#define _TAH_GuiWidget_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/Vector2i.h>

namespace tas
{

/* Widget type */
#define WT_WINDOW       1
#define WT_LABEL        2
#define WT_BUTTON       3
#define WT_EDIT         4
#define WT_CHECKBOX     5
#define WT_COMBOBOX     6
#define WT_STATUSBAR    7
#define WT_TOOLBAR      8
#define WT_LISTVIEW     9
#define WT_PROGRESSBAR  10

/** Base class for Windows, Buttons .. */
class Widget
{
protected:
	HWND mHandle;
	Vector2i mPosition;
	Vector2i mSize;
	String mCaption;
	bool mActivate;
	bool mEnable;
	bool mShow;
	bool mFocus;
	bool mMaximize;
	uint mId;
	Widget* mParent;
	uint mType; /// widget
public:

	Widget();
	virtual ~Widget();

	void setHandle(HWND handle);
	void setId(uint id);
	void setPosition(Vector2i position);
	void setSize(Vector2i size);
	void setCaption(const String& caption);
	void setParent(Widget* parent);
	void setActivate(bool state);
	void setFocused(bool state);
	void setMaximized(bool state);

	HWND getHandle();
	uint getId();
	uint getType();
	Vector2i getPosition();
	Vector2i getSize();
	String getCaption();
	Widget* getParent();

	/// create and display widget on screen
	virtual bool create() = 0;

	/// event resize
	virtual void onSize() {}

	/// create window implementation
	void createWindow(const String& className, uint style);

	/// tooltip for child widget
	bool createTooltip(const String& tooltip);

	bool activate();
	bool isActivated();

	bool enable(bool state);
	bool isEnabled();

	bool show(bool state);
	bool isShown();

	bool setFocus();
	bool isFocused();

	bool maximize(bool state);
	bool isMaximized();

	bool setForeground();
	void close();
	void destroy();
	void move(Vector2i position, Vector2i size);

	/// font : HFONT
	bool setFont(void* font);
	void* getFont();
};

}

#endif