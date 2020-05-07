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
#include <Widget/Edit.h>
#include <Widget/Window.h>

namespace tas
{

Edit::Edit()
{
	mPassword = 0;
	mType = WT_EDIT;
}

Edit::Edit(Window* parent, const String& caption, uint id, bool password, Vector2i size, Vector2i position)
{
	mPassword = password;
	mType = WT_EDIT;
	mParent = parent;
	parent->registerWidget(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Edit::~Edit()
{
	mParent = 0;
}

bool Edit::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER;
	if(mPassword)
		wstyle |= ES_PASSWORD;
	createWindow("Edit", wstyle);
	return 1;
}

bool Edit::recreate(bool password)
{
	mPassword = password;
	String text;
	text.reserve(256);
	getText(text);
	setCaption(text);
	SendMessage(mHandle, WM_CLOSE, 0, 0);
	create();
	return 1;
}

void Edit::setPassword(bool state)
{
	mPassword = state;
}

void Edit::setText(const String& string)
{
	SendMessage(mHandle, WM_SETTEXT, 0, (LPARAM) string.p());
}

void Edit::getText(String& string)
{
	SendMessage(mHandle, WM_GETTEXT, (WPARAM) string.capacity(), (LPARAM) string.p());
	string.update();
}

}