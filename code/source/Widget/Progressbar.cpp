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
#include <Widget/Progressbar.h>
#include <Widget/Window.h>
#include <CommCtrl.h>

namespace tas
{

Progressbar::Progressbar()
{
	mType = WT_PROGRESSBAR;
}

Progressbar::Progressbar(Window* parent, const String& caption, uint id, Vector2i size, Vector2i position)
{
	mType = WT_PROGRESSBAR;
	mParent = parent;
	parent->registerWidget(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Progressbar::~Progressbar()
{
	mParent = 0;
}

bool Progressbar::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE;
	createWindow(PROGRESS_CLASS, wstyle);
	return 1;
}

void Progressbar::setPositionBar(uint pos)
{
	SendMessage(mHandle, PBM_SETPOS, pos, 0);
}

uint Progressbar::getPositionBar()
{
	return SendMessage(mHandle, PBM_GETPOS, 0, 0);
}

void Progressbar::setRange(uint range)
{
	SendMessage(mHandle, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, range));
}

}