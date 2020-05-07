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
#include <Widget/Application.h>
#include <Widget/Window.h>
#include <Widget/Event.h>
#include <Shellapi.h>

namespace tas
{

Window::Window()
{
	mType = WT_WINDOW;
	mIconId = -1;
	GuiApplication::ptr()->registerWindow(this);
	mWidgetCallback = 0;
	mIdleCallback = 0;
	mStop = 0;
}

Window::Window(Window* parent, const String& caption, uint id, Vector2i size, Vector2i position,
               WidgetCallback callback, IdleCallback idleCallback)
{
	mType = WT_WINDOW;
	mIconId = -1;
	mProperties = 0;
	mStop = 0;
	mParent = parent;
	mWidgetCallback = callback;
	mIdleCallback = idleCallback;
	GuiApplication::ptr()->registerWindow(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Window::~Window()
{
	_printf("Destructor %u\n", mId);
	mWidgets.clear_ptr();
}

bool Window::create()
{
	bool menu = (mProperties & 1) != 0;
	bool maximize = (mProperties & 2) != 0;
	bool dialog = (mProperties & 4) != 0;
	bool alignCenter = (mProperties & 8) != 0;

	String className;
	className.format("WIDGET_GUI_%u", mId);

	HINSTANCE hinstance = GetModuleHandle(0);
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = hinstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = className.p();
	wc.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(240,240,240));
	if(mIconId != -1)
		wc.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(mIconId));
	RegisterClass(&wc);

	uint windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	if(maximize)
		windowStyle |= WS_MAXIMIZE;
	if(dialog)
		windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_POPUP;

	// centered main window position
	int rc[4] = {0, 0, mSize.x, mSize.y};
	AdjustWindowRect((LPRECT)rc, windowStyle, menu);
	Vector2i winSize(rc[2] - rc[0], rc[3] - rc[1]);

	Vector2i deskSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	forn(2)
	{
		if(mPosition[i] > deskSize[i] - winSize[i] or mPosition[i] < 0)
		{
			alignCenter = 1;
			break;
		}
	}

	if(alignCenter)
	{
		mPosition = (deskSize - winSize) / 2;
	}

	HWND parent = 0;
	if(mParent)
	{
		parent = mParent->getHandle();
	}

	// create main window
	mHandle = CreateWindow(className.p(), mCaption.p(), windowStyle,
	                       mPosition.x, mPosition.y, winSize.x, winSize.y,
	                       parent, NULL, hinstance, NULL);

	mShow = 1;
	return 1;
}

bool Window::run()
{
	// while(SystemMessage() and !mStop)
	while(!mStop)
	{
		SystemMessage();
		if(mIdleCallback) mIdleCallback();
	}
	_printf("Window stopped %u\n", mId);
	return 1;
}

bool Window::stop()
{
	mStop = 1;
	return 1;
}

void Window::createEx(uint properties, int iconId)
{
	mProperties = properties;
	mIconId = iconId;
	create();
}

void Window::createWidgets()
{
	forn(mWidgets.size())
	mWidgets[i]->create();
}

void Window::setProperties(uint properties)
{
	mProperties = properties;
}

void Window::setIconId(int iconId)
{
	mIconId = iconId;
}

void Window::setDragDrop(bool state)
{
	DragAcceptFiles(mHandle, state);
}

void Window::alignByCenter()
{
	bool menu = (mProperties & 1) != 0;
	bool dialog = (mProperties & 4) != 0;
	Vector2i deskSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	uint windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	if(dialog)
		windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_POPUP;
	int rc[4] = {0, 0, mSize.x, mSize.y};
	AdjustWindowRect((LPRECT)rc, windowStyle, menu);
	Vector2i winSize(rc[2] - rc[0], rc[3] - rc[1]);
	mPosition = (deskSize - winSize) / 2;
	move(mPosition, winSize);
}

void Window::registerWidget(Widget* widget)
{
	mWidgets.push_back(widget);
}

void Window::unregisterWidget(Widget* widget)
{
	int n = mWidgets.find(widget);
	if(n != -1)
	{
		mWidgets.erase(n);
		safe_delete(widget);
	}
}

Widget* Window::findWidgetById(uint id)
{
	forn(mWidgets.size())
	{
		if(mWidgets[i]->getId() == id)
			return mWidgets[i];
	}
	return 0;
}

Widget* Window::findWidgetByType(uint type)
{
	forn(mWidgets.size())
	{
		if(mWidgets[i]->getType() == type)
			return mWidgets[i];
	}
	return 0;
}

WidgetCallback Window::getWidgetCallback()
{
	return mWidgetCallback;
}

void Window::setSizeMin(Vector2i size)
{
	mSizeMin = size;
}

Vector2i Window::getSizeMin()
{
	return mSizeMin;
}

void Window::onSize()
{
	forn(mWidgets.size())
	mWidgets[i]->onSize();
}

}