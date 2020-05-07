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
// #include <Common/platform/swindows.h>
#include <windows.h>
#include <Common/StringLib.h>
#include <CommCtrl.h>
#include <Widget/Listview.h>
#include <Widget/Imagelist.h>
#include <Widget/Window.h>

namespace tas
{

Listview::Listview()
{
	mType = WT_LISTVIEW;
}

Listview::Listview(Window* parent, uint id, Vector2i size, Vector2i position)
{
	mType = WT_LISTVIEW;
	mParent = parent;
	parent->registerWidget(this);
	setId(id);
	setSize(size);
	setPosition(position);
}

Listview::~Listview()
{
}

bool Listview::create()
{
	uint wstyle = LVS_REPORT | WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_EDITLABELS;
	createWindow(WC_LISTVIEW, wstyle);
	return 1;
}

void Listview::setColumn(const String& name, uint iCol, uint width, int fmt, int type)
{
	LVCOLUMN LvCol;
	menset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
	LvCol.fmt = fmt; // LVCFMT_LEFT
	LvCol.pszText = name.p();
	LvCol.cx = width;
	uint msg = type ? LVM_SETCOLUMN : LVM_INSERTCOLUMN;
	SendMessage(mHandle, msg, iCol, (LPARAM)&LvCol);
}

void Listview::insertItem(uint iStr)
{
	LVITEM LvItem;
	menset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask = LVIF_TEXT;
	LvItem.pszText = L"";
	LvItem.cchTextMax = 32;
	LvItem.iItem = iStr;
	LvItem.iSubItem = 0;
	SendMessage(mHandle, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
}

void Listview::setItem(const String& string, uint iStr, uint iCol, int iImage)
{
	LVITEM LvItem;
	menset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	LvItem.pszText = string.p();
	LvItem.cchTextMax = string.capacity();
	LvItem.iItem = iStr;
	LvItem.iSubItem = iCol;
	LvItem.iImage = iImage;
	SendMessage(mHandle, LVM_SETITEM, 0, (LPARAM)&LvItem);
}

void Listview::deleteColumn(uint iCol)
{
	SendMessage(mHandle, LVM_DELETECOLUMN, iCol, (LPARAM)0);
}

void Listview::deleteItem(uint iStr)
{
	SendMessage(mHandle, LVM_DELETEITEM, iStr, 0);
}

void Listview::deleteAllItems()
{
	SendMessage(mHandle, LVM_DELETEALLITEMS, 0, 0);
}

void Listview::setItemState(int iStr, uint state, uint mask)
{
	ListView_SetItemState(mHandle, iStr, state, mask);
}

void Listview::selectItem(int iStr)
{
	setItemState(iStr, 1, 1); // focused
	setItemState(iStr, 2, 2); // select
}

void Listview::unselectItem(int iStr)
{
	setItemState(iStr, 0, 1); // defocused
	setItemState(iStr, 0, 2); // deselect
}

void Listview::selectAll()
{
	setItemState(-1, 1, 1); // focused all strings
	setItemState(-1, 2, 2); // select  all strings
}

void Listview::unselectAll()
{
	setItemState(-1, 0, 1); // defocused all strings
	setItemState(-1, 0, 2); // deselect  all strings
}

int Listview::getColumnWidth(uint iCol)
{
	LVCOLUMN LvCol;
	LvCol.mask = LVCF_WIDTH;
	SendMessage(mHandle, LVM_GETCOLUMN, iCol, (LPARAM)&LvCol);
	return LvCol.cx;
}

int Listview::getItemCount(int type)
{
	uint msg = type ? LVM_GETSELECTEDCOUNT : LVM_GETITEMCOUNT;
	return SendMessage(mHandle, msg, 0, 0);
}

int Listview::getSelectItem(int iStr)
{
	// ret -1 if not selected string
	// iStr -1 first
	return ListView_GetNextItem(mHandle, iStr, LVNI_SELECTED);
}

void Listview::editLabel(uint iStr)
{
	SendMessage(mHandle, LVM_EDITLABEL, iStr, (LPARAM)0);
}

void Listview::setExtendedStyle(uint style)
{
	uint extendStyle = 0;
	if(style & 1)
		extendStyle |= LVS_EX_DOUBLEBUFFER;
	if(style & 2)
		extendStyle |= LVS_EX_FULLROWSELECT;
	SendMessage(mHandle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, extendStyle);
}

void Listview::setImagelist(Imagelist* imagelist)
{
	ListView_SetImageList(mHandle, imagelist->getHandle(), LVSIL_SMALL);
}

void Listview::showArrow(uint iCol, int showArrow)
{
	// arrow 0 disable, 1 down, 2 up
	HWND    hHeader  = NULL;
	HDITEM  hdrItem  = {0};

	hHeader = ListView_GetHeader(mHandle);

	if(hHeader)
	{
		hdrItem.mask = HDI_FORMAT;

		if(Header_GetItem(hHeader, iCol, &hdrItem))
		{
			if(showArrow == 2) // SHOW_UP_ARROW
			{
				hdrItem.fmt = (hdrItem.fmt & ~HDF_SORTDOWN) | HDF_SORTUP;
			}
			else if(showArrow == 1) // SHOW_DOWN_ARROW
			{
				hdrItem.fmt = (hdrItem.fmt & ~HDF_SORTUP) | HDF_SORTDOWN;
			}
			else
			{
				hdrItem.fmt = hdrItem.fmt & ~(HDF_SORTDOWN | HDF_SORTUP);
			}
			Header_SetItem(hHeader, iCol, &hdrItem);
		}
	}
}

void Listview::onEvent(uint message, uint_t param)
{
	switch(message)
	{
	case 0:
		break;
	case 1:
		break;
	}
}

}