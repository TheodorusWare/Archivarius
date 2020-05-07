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
#include <Common/StringLib.h>
#include <Widget/Menu.h>
#include <Widget/Window.h>

namespace tas
{

Menu::Menu()
{
	mHandle = 0;
}

Menu::~Menu()
{
}

bool Menu::create(bool popup)
{
	if(!popup)
		mHandle = CreateMenu();
	else
		mHandle = CreatePopupMenu();
	return 1;
}

bool Menu::destroy()
{
	DestroyMenu(mHandle);
	return 1;
}

bool Menu::appendMenu(Menu* menu, const String& caption)
{
	AppendMenu(mHandle, MF_STRING | MF_POPUP, (UINT)menu->getHandle(), caption.p());
	return 1;
}

bool Menu::appendItem(uint id, const String& caption)
{
	AppendMenu(mHandle, MF_STRING, id, caption.p());
	return 1;
}

bool Menu::appendSeparator()
{
	AppendMenu(mHandle, MF_SEPARATOR, 0, L"");
	return 1;
}

bool Menu::attach(Window* window)
{
	SetMenu(window->getHandle(), mHandle);
	return 1;
}

bool Menu::trackMenu(Window* window, Vector2i position, uint flags)
{
	return TrackPopupMenu(mHandle, flags, position.x, position.y, 0, window->getHandle(), NULL);
}

HMENU Menu::getHandle()
{
	return mHandle;
}

}