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
#ifndef _TAH_GuiWindow_h_
#define _TAH_GuiWindow_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/container/Array.h>
#include <Common/Vector2i.h>
#include <Widget/Widget.h>
#include <Widget/Event.h>

namespace tas
{

class Window: public Widget
{
	Array<Widget*> mWidgets;
	uint mProperties; // for creating window, combine bit flags
	// 1 have menu, 2 maximized, 4 dialog type, 8 align center
	int mIconId;
	WidgetCallback mWidgetCallback;
	IdleCallback mIdleCallback;
	Vector2i mSizeMin;
	bool mStop;
public:
	Window();
	Window(Window* parent, const String& caption, uint id, Vector2i size, Vector2i position,
	       WidgetCallback callback, IdleCallback idleCallback);
	~Window();
	bool create();
	bool run();
	bool stop();
	void createEx(uint properties, int iconId);
	void createWidgets();
	void setProperties(uint properties);
	void setIconId(int iconId); // resource id
	void setDragDrop(bool state);
	void alignByCenter();
	void registerWidget(Widget* widget);
	void unregisterWidget(Widget* widget);
	Widget* findWidgetById(uint id);
	Widget* findWidgetByType(uint type);
	WidgetCallback getWidgetCallback();
	void setSizeMin(Vector2i size);
	Vector2i getSizeMin();

	/// event resize
	void onSize();
};

}

#endif