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
#include <Widget/Label.h>
#include <Widget/Window.h>

namespace tas
{

Label::Label()
{
	mType = WT_LABEL;
	mStyle = 0;
}

Label::Label(Window* parent, const String& caption, uint id, uint style, Vector2i size, Vector2i position)
{
	mType = WT_LABEL;
	mStyle = style;
	mParent = parent;
	parent->registerWidget(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Label::~Label()
{
}

bool Label::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE | mStyle;
	createWindow("Static", wstyle);
	return 1;
}

void Label::setText(const String& string)
{
	SendMessage(mHandle, WM_SETTEXT, 0, (LPARAM) string.p());
}

void Label::getText(String& string)
{
	SendMessage(mHandle, WM_GETTEXT, (WPARAM) string.capacity(), (LPARAM) string.p());
	string.update();
}

void Label::setImage(void* image)
{
	SendMessage(mHandle, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image);
}

void Label::setImageIcon(void* image)
{
	SendMessage(mHandle, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)image);
}

}