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
#ifndef _TAH_GuiEvent_h_
#define _TAH_GuiEvent_h_

#include <Common/Config.h>

namespace tas
{

class Widget;

/* Window messages */
#define EVM_CREATE     0x01
#define EVM_CLOSE      0x02
#define EVM_DESTROY    0x03
#define EVM_SIZE       0x04
#define EVM_ACTIVATE   0x05

/* Button messages */
#define EVM_CLICKED    0x32

/* Drag drop */
#define EVM_DROP       0x35

/* Context menu */
#define EVM_CONTEXTMENU    0x36

/* Combobox messages */
#define EVM_CB_SELCHANGE   0x41
#define EVM_CB_ENDEDIT     0x42
#define EVM_CB_KILLFOCUS   0x43
#define EVM_CB_DROP_SHOW   0x44
#define EVM_CB_DROP_CLOSE  0x45

/* Listview messages */
#define EVM_LIST_SETFOCUS   0x50
#define EVM_LIST_KILLFOCUS  0x51
#define EVM_LIST_DBLCLK     0x52
#define EVM_LIST_COLCLK     0x53
#define EVM_LIST_UPDATE     0x54
#define EVM_LIST_BEGEDIT    0x55
#define EVM_LIST_ENDEDIT    0x56
#define EVM_LIST_KEYDOWN    0x57

/* LOWORD, HIWORD 2 byte */
#define WT_LOWORD(l) (l & 0xFFFF)
#define WT_HIWORD(l) ((l >> 16) & 0xFFFF)

/* set command to application from window procedure */
#define WT_SEND_MESSAGE(c, p) GuiApplication::ptr()->setMessage(c, (uint_t)p)

/* Callback procedures */
typedef int (*WidgetCallback) (Widget* widget, uint message, uint_t param);
typedef void (*IdleCallback) ();

/* Implementaion main window procedure */
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/* Processed system messages */
bool SystemMessage();

/* Send Message to Application */
int _SendMessage(uint message, uint_t param);

}

#endif