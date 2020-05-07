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
#include <Widget/Button.h>
#include <Widget/Window.h>

namespace tas
{

Button::Button()
{
	mType = WT_BUTTON;
}

Button::Button(Window* parent, const String& caption, uint id, Vector2i size, Vector2i position)
{
	mType = WT_BUTTON;
	mParent = parent;
	parent->registerWidget(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Button::~Button()
{
	mParent = 0;
}

bool Button::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE | BS_NOTIFY;
	createWindow("Button", wstyle);
	return 1;
}

}