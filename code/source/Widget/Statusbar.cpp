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
#include <Widget/Statusbar.h>
#include <Widget/Window.h>
#include <CommCtrl.h>

namespace tas
{

Statusbar::Statusbar()
{
	mType = WT_STATUSBAR;
}

Statusbar::Statusbar(Window* parent, uint id)
{
	mType = WT_STATUSBAR;
	mParent = parent;
	parent->registerWidget(this);
	setId(id);
}

Statusbar::~Statusbar()
{
	mParent = 0;
}

bool Statusbar::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP;
	createWindow(STATUSCLASSNAME, wstyle);

	int rc[4] = {0};
	SendMessage(mHandle, SB_GETRECT,  (WPARAM)0, (LPARAM)rc);
	mSize.y = rc[3] - rc[1];

	return 1;
}

bool Statusbar::setParts(int* parts, int count)
{
	SendMessage(mHandle, SB_SETPARTS, (WPARAM)count, (LPARAM)parts);
	return 1;
}

bool Statusbar::setText(int i, const String& text)
{
	// _printf("sbar h %X\n", mHandle);
	SendMessage(mHandle, SB_SETTEXT,  (WPARAM)i, (LPARAM)text.p());
	return 1;
}

void Statusbar::onSize()
{
	move(Vector2i(), Vector2i());
}

}