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
#ifndef _TAH_GuiListview_h_
#define _TAH_GuiListview_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/Vector2i.h>
#include <Widget/Widget.h>

namespace tas
{

class Window;
class Imagelist;

class Listview: public Widget
{
public:
	Listview();
	Listview(Window* parent, uint id, Vector2i size, Vector2i position);
	~Listview();
	bool create();

	void setColumn(const String& name, uint iCol, uint width, int fmt, int type);
	void insertItem(uint iStr);
	void setItem(const String& string, uint iStr, uint iCol, int iImage);
	void deleteColumn(uint iCol);
	void deleteItem(uint iStr);
	void deleteAllItems();
	void setItemState(int iStr, uint state, uint mask);
	void selectItem(int iStr);
	void unselectItem(int iStr);
	void selectAll();
	void unselectAll();
	int  getColumnWidth(uint iCol);
	int  getItemCount(int type);
	int  getSelectItem(int iStr);
	void editLabel(uint iStr);
	void setExtendedStyle(uint style);
	void setImagelist(Imagelist* imagelist);
	void showArrow(uint iCol, int arrow);

	// notify message
	void onEvent(uint message, uint_t param);
};

}

#endif