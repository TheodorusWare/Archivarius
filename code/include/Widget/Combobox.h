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
#ifndef _TAH_GuiCombobox_h_
#define _TAH_GuiCombobox_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/Vector2i.h>
#include <Widget/Widget.h>

namespace tas
{

class Window;
class Imagelist;

class Combobox: public Widget
{
public:
	uint mStyle; // flags; 1 : extend style; 2 : with edit string; 4 : kill focus after select ok
	int mSelectItem; // for notify begin edit
	bool mDropdown;

	Combobox();
	Combobox(Window* parent, const String& caption, uint id, uint style, Vector2i size, Vector2i position);
	~Combobox();

	bool create();

	// for simple style, not extended
	void appendString(const String& str);
	void updateString(int i, const String& str);
	void deleteString(int i);

	/* extended style
		setItem type 0 insert, 1 set
	*/
	void setItem(const String& str, int item, int image, int indent, int type);
	void deleteItem(int item);
	void setImagelist(Imagelist* imagelist);

	void deleteAllItems();
	void setCurrentItem(int i);
	int  getCurrentItem();
	void showDropdown(int state);
	void getText(String& string);

	// notify message
	void onEvent(uint message, uint_t param);
};

}

#endif