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
// #include <Common/StringLib.h>
#include <CommCtrl.h>
#include <Widget/Widget.h>
#include <Widget/Window.h>

namespace tas
{

Widget::Widget()
{
	mHandle = 0;
	mId = 0;
	mParent = 0;
	mType = 0;
	mEnable = 1;
	mShow = 0;
	mFocus = 0;
	mMaximize = 0;
	mActivate = 0;
}

Widget::~Widget()
{
	mHandle = 0;
}

void Widget::setHandle(HWND handle)
{
	mHandle = handle;
}

void Widget::setId(uint id)
{
	mId = id;
}

void Widget::setPosition(Vector2i position)
{
	mPosition = position;
}

void Widget::setSize(Vector2i size)
{
	mSize = size;
}

void Widget::setCaption(const String& caption)
{
	mCaption = caption;
}

void Widget::setParent(Widget* parent)
{
	mParent = parent;
}

void Widget::setActivate(bool state)
{
	mActivate = state;
}

void Widget::setFocused(bool state)
{
	mFocus = state;
}

void Widget::setMaximized(bool state)
{
	mMaximize = state;
}

HWND Widget::getHandle()
{
	return mHandle;
}

uint Widget::getId()
{
	return mId;
}

uint Widget::getType()
{
	return mType;
}

Vector2i Widget::getPosition()
{
	return mPosition;
}

Vector2i Widget::getSize()
{
	return mSize;
}

String Widget::getCaption()
{
	return mCaption;
}

Widget* Widget::getParent()
{
	return mParent;
}

bool Widget::activate()
{
	mActivate = 1;
	SetActiveWindow(mHandle);
	return 1;
}

bool Widget::isActivated()
{
	return mActivate;
}

bool Widget::enable(bool state)
{
	// if(!state)
	// mActivate = 0;
	mEnable = state;
	EnableWindow(mHandle, mEnable);
	return 1;
}

bool Widget::isEnabled()
{
	return mEnable;
}

bool Widget::show(bool state)
{
	mShow = state;
	ShowWindow(mHandle, state ? SW_SHOW : SW_HIDE);
	return 1;
}

bool Widget::isShown()
{
	return mShow;
}

bool Widget::setForeground()
{
	SetForegroundWindow(mHandle);
	return 1;
}

bool Widget::setFocus()
{
	mFocus = 1;
	SetFocus(mHandle);
	return 1;
}

bool Widget::isFocused()
{
	return mFocus;
}

bool Widget::maximize(bool state)
{
	ShowWindow(mHandle, state ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	return mMaximize;
}

bool Widget::isMaximized()
{
	return mMaximize;
}

void Widget::createWindow(const String& className, uint style)
{
	HINSTANCE hinstance = GetModuleHandle(0);
	HWND parent = 0;
	if(mParent) parent = mParent->getHandle();
	mHandle = CreateWindow(className.p(), mCaption.p(), style,
	                       mPosition.x, mPosition.y, mSize.x, mSize.y,
	                       parent, (HMENU) mId, hinstance, NULL);
	// HFONT fontSystem = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	// setFont(fontSystem);
}

bool Widget::createTooltip(const String& tooltip)
{
	if(!mParent or !mHandle) return 0;
	HINSTANCE hinstance = GetModuleHandle(0);
	HWND hTip = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP,
	                         CW_USEDEFAULT, CW_USEDEFAULT,
	                         CW_USEDEFAULT, CW_USEDEFAULT,
	                         mParent->getHandle(), NULL,
	                         hinstance, NULL);
	if(!hTip)
		return 0;
	TOOLINFO toolInfo = {0};
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = mParent->getHandle();
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)mHandle;
	toolInfo.lpszText = tooltip.p();
	SendMessage(hTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
	return 1;
}

void Widget::close()
{
	SendMessage(mHandle, WM_CLOSE, 0, 0);
	if(mParent and mType > 1)
		((Window*)mParent)->unregisterWidget(this);
}

void Widget::destroy()
{
	DestroyWindow(mHandle);
	if(mParent and mType > 1)
		((Window*)mParent)->unregisterWidget(this);
}

void Widget::move(Vector2i pos, Vector2i size)
{
	MoveWindow(mHandle, pos.x, pos.y, size.x, size.y, 1);
}

bool Widget::setFont(void* font)
{
	if(!font) return 0;
	SendMessage(mHandle, WM_SETFONT, (WPARAM)font, 0);
	return 1;
}

void* Widget::getFont()
{
	return (void*) SendMessage(mHandle, WM_GETFONT, 0, 0);
}

}