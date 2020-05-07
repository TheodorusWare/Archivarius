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
#ifndef _TAH_GuiToolbar_h_
#define _TAH_GuiToolbar_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/container/Array.h>
#include <Common/Vector2i.h>
#include <Widget/Widget.h>

namespace tas
{

class Window;
class Imagelist;

class ToolbarButton
{
public:
	uint mId;
	uint mIconId;
	String mCaption;
	String mTooltip;
	ToolbarButton();
	ToolbarButton(uint id, uint iconId, const String& caption, const String& tooltip);
	~ToolbarButton();
};

class Toolbar: public Widget
{
	Vector2i mButtonSize;
	Array<ToolbarButton*> mButtons;
	Imagelist* mImagelist;
	bool mTooltipStyle; // style
public:
	Toolbar();
	Toolbar(Window* parent, uint id, Vector2i buttonSize, bool tooltipStyle, Imagelist* imageList);
	~Toolbar();

	bool create();

	void appendButton(ToolbarButton* button);
	void createButtons();
	ToolbarButton* findButtonById(uint id);

	/// event resize
	void onSize();
};

}

#endif