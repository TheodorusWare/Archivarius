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
#include <CommCtrl.h>
#include <Common/StringLib.h>
#include <Widget/Combobox.h>
#include <Widget/Window.h>
#include <Widget/Imagelist.h>

namespace tas
{

Combobox::Combobox()
{
	mType = WT_COMBOBOX;
	mStyle = 0;
	mSelectItem = 0;
	mDropdown = 0;
}

Combobox::Combobox(Window* parent, const String& caption, uint id, uint style, Vector2i size, Vector2i position)
{
	mDropdown = 0;
	mSelectItem = 0;
	mStyle = style;
	mType = WT_COMBOBOX;
	mParent = parent;
	parent->registerWidget(this);
	setCaption(caption);
	setId(id);
	setSize(size);
	setPosition(position);
}

Combobox::~Combobox()
{
	mParent = 0;
}

bool Combobox::create()
{
	// wstyle = CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL;
	// CBS_DROPDOWN | WS_CHILD | WS_VISIBLE
	/*
		for extended style window size y is height drop down list
	*/

	uint wstyle = WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL;
	if(mStyle & 2)
		wstyle |= CBS_DROPDOWN;
	else
		wstyle |= CBS_DROPDOWNLIST;
	const wchar_t* className = (mStyle & 1) ? WC_COMBOBOXEX : WC_COMBOBOX;
	createWindow(className, wstyle);

	int rc[4] = {0};
	GetWindowRect(mHandle, (LPRECT)rc);
	// mSize.x = rc[2] - rc[0];
	mSize.y = rc[3] - rc[1];
	// _printf("\ncb size %u %u menu y %u\n\n", mSize.x, mSize.y, GetSystemMetrics(SM_CYMENUSIZE));

	return 1;
}

void Combobox::appendString(const String& str)
{
	SendMessage(mHandle, CB_ADDSTRING, 0, (LPARAM) str.p());
}

void Combobox::updateString(int i, const String& str)
{
	SendMessage(mHandle, CB_DELETESTRING, (WPARAM) i, 0);
	SendMessage(mHandle, CB_INSERTSTRING, (WPARAM) i, (LPARAM) str.p());
}

void Combobox::deleteString(int i)
{
	SendMessage(mHandle, CB_DELETESTRING, (WPARAM) i, 0);
}

void Combobox::setItem(const String& str, int item, int image, int indent, int type)
{
	COMBOBOXEXITEM cbItem;
	menset(&cbItem, 0, sizeof(cbItem));
	cbItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT;
	cbItem.iItem = item;
	cbItem.iImage = image;
	cbItem.iSelectedImage = image;
	cbItem.iIndent = indent;
	cbItem.pszText = str.p();
	cbItem.cchTextMax = str.length() + 2;
	uint msg = type ? CBEM_SETITEM : CBEM_INSERTITEM;
	SendMessage(mHandle, msg, 0, (LPARAM) &cbItem);
	// _printf("txt %s %u\n", cbItem.pszText, cbItem.cchTextMax);
}

void Combobox::deleteItem(int item)
{
	SendMessage(mHandle, CBEM_DELETEITEM, item, 0);
}

void Combobox::setImagelist(Imagelist* imagelist)
{
	SendMessage(mHandle, CBEM_SETIMAGELIST, 0, (LPARAM)imagelist->getHandle());
}

void Combobox::deleteAllItems()
{
	SendMessage(mHandle, CB_RESETCONTENT, 0, 0);
}

void Combobox::setCurrentItem(int i)
{
	SendMessage(mHandle, CB_SETCURSEL, (WPARAM) i, 0);
}

int Combobox::getCurrentItem()
{
	return SendMessage(mHandle, CB_GETCURSEL, 0, 0);
}

void Combobox::showDropdown(int state)
{
	SendMessage(mHandle, CB_SHOWDROPDOWN, (WPARAM) state, 0);
}

void Combobox::getText(String& string)
{
	SendMessage(mHandle, WM_GETTEXT, (WPARAM) string.capacity(), (LPARAM) string.p());
	string.update();
}

void Combobox::onEvent(uint message, uint_t param)
{
	switch(message)
	{
	case 0:
		mFocus = 1;
		mSelectItem = getCurrentItem();
		break;
	case 1:
		mSelectItem = param;
		break;
	case 2:
		mFocus = 0;
		enable(0);
		enable(1);
		setCurrentItem(mSelectItem);
		break;
	case 3:
		setCurrentItem(mSelectItem);
		// mDropdown = 1;
		break;
	case 4:
		// mDropdown = 0;
		break;
	}

	// _printf("cb onEvent %u \n", message, mSelectItem);
}

}