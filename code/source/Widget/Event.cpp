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
#include <Widget/Event.h>
#include <Widget/Window.h>
#include <Widget/Listview.h>
#include <Widget/Combobox.h>
#include <Widget/Toolbar.h>
#include <CommCtrl.h>

#define _printf noop

namespace tas
{

static WidgetCallback pWidgetCallback = 0;

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	pWidgetCallback = 0;

	Window* window = GuiApplication::ptr()->findWindowByHandle(hwnd);
	if(!window and message == WM_CREATE)
	{
		window = GuiApplication::ptr()->getLastWindow();
		if(window)
			window->setHandle(hwnd);
	}

	if(!window)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	pWidgetCallback = window->getWidgetCallback();

	switch(message)
	{

	case WM_CREATE:
		_printf("WM_CREATE %u\n", window->getId());
		window->createWidgets();
		pWidgetCallback(window, EVM_CREATE, 0);
		break;

	case WM_CLOSE:
	{
		_printf("WM_CLOSE %u\n", window->getId());
		int ret = pWidgetCallback(window, EVM_CLOSE, 0);
		if(!ret) return 0;
		break;
	}

	case WM_DESTROY:
		_printf("WM_DESTROY %u\n", window->getId());
		pWidgetCallback(window, EVM_DESTROY, 0);
		GuiApplication::ptr()->unregisterWindow(window);
		_printf("PostQuitMessage\n");
		window->stop();
		break;

	case WM_DROPFILES:
		_printf("WM_DROPFILES\n");
		pWidgetCallback(window, EVM_DROP, (uint_t)wParam);
		break;

	case WM_COMMAND:
	{
		half notifyCode = HIWORD(wParam);
		half id = LOWORD(wParam);
		_printf("WM_COMMAND code %u\n", notifyCode);

		switch(notifyCode)
		{
		case BN_CLICKED:
		{
			_printf("BN_CLICKED %u %u\n", window->getId(), id);
			pWidgetCallback(window, EVM_CLICKED, id);
			break;
		}

		case EN_SETFOCUS:
		{
			Widget* edit = window->findWidgetById(id);
			if(edit)
				edit->setFocused(1);
			_printf("EN_SETFOCUS %u %u\n", window->getId(), id);
			break;
		}

		case EN_KILLFOCUS:
		{
			Widget* edit = window->findWidgetById(id);
			if(edit)
				edit->setFocused(0);
			_printf("EN_KILLFOCUS %u %u\n", window->getId(), id);
			break;
		}

		// combobox dropdown list focus
		case CBN_SETFOCUS:
		{
			_printf("CBN_SETFOCUS %u %u\n", window->getId(), id);
			break;
		}

		case CBN_KILLFOCUS:
		{
			_printf("CBN_KILLFOCUS %u %u\n", window->getId(), id);
			break;
		}

		// show dropdown
		case CBN_DROPDOWN:
		{
			Widget* widget = window->findWidgetById(id);
			if(widget->getType() == WT_COMBOBOX)
			{
				_printf("CBN_DROPDOWN %u\n", id);
				((Combobox*) widget)->onEvent(0, 0); // save cur selection
				pWidgetCallback(window, EVM_CB_DROP_SHOW, id);
			}
			break;
		}

		// close dropdown
		case CBN_CLOSEUP:
		{
			Widget* widget = window->findWidgetById(id);
			if(widget->getType() == WT_COMBOBOX)
				pWidgetCallback(window, EVM_CB_DROP_CLOSE, id);
			break;
		}

		case CBN_SELENDOK:
		{
			Combobox* combobox = (Combobox*) window->findWidgetById(id);
			if(combobox)
			{
				int curItem = combobox->getCurrentItem();
				int selItem = combobox->mSelectItem;
				_printf("CBN_SELENDOK %u %u\n", curItem,  selItem);
				combobox->onEvent(0, 0); // save cur selection
				if(combobox->mStyle & 4)
					WT_SEND_MESSAGE(1, combobox); // kill focus
				if(curItem != selItem)
					pWidgetCallback(window, EVM_CB_SELCHANGE, (uint_t) id);
				if(combobox->mStyle & 4)
					pWidgetCallback(window, EVM_CB_KILLFOCUS, (uint_t) id);
			}
			break;
		}
		case CBN_SELENDCANCEL:
		{
			// close drop down
			Combobox* combobox = (Combobox*) window->findWidgetById(id);
			if(combobox and combobox->isFocused())
				WT_SEND_MESSAGE(2, combobox); // restore cur selection
			break;
		}

			/*
			case CBN_SELCHANGE:
			{
				// pWidgetCallback(window, EVM_CB_SELCHANGE, (uint_t) id);
				Combobox* combobox = (Combobox*) window->findWidgetById(id);
				_printf("CBN_SELCHANGE %u %u %u\n", window->getId(), id, combobox->getCurrentItem());
				if(combobox)
				{
					// combobox->onEvent(0, 0); // save cur selection
					if(combobox->getKillFocus())
					{
						// pWidgetCallback(window, EVM_CB_SELCHANGE, (uint_t) id);
						// combobox->onEvent(0, 0); // save cur selection
						// WT_APPCMD(1, combobox); // kill focus
					}
				}
				break;
			}
			*/
		}
		break;
	}

	case WM_ACTIVATE:
	{
		int state = WT_LOWORD(wParam) and WT_HIWORD(wParam) == 0;
		_printf("WM_ACTIVATE win %u state %u low %u\n", window->getId(), state, WT_LOWORD(wParam));
		window->setActivate(state != 0);
		pWidgetCallback(window, EVM_ACTIVATE, (uint_t)(state != 0));
		break;
	}

	case WM_SIZE:
	{
		Vector2i size(WT_LOWORD(lParam), WT_HIWORD(lParam));
		_printf("WM_SIZE %u sz %u %u\n", window->getId(), size.x, size.y);
		window->setSize(Vector2i(size.x, size.y));
		window->onSize();
		window->setMaximized(wParam == 2);
		pWidgetCallback(window, EVM_SIZE, (uint_t)lParam);
		break;
	}

	case WM_WINDOWPOSCHANGED:
	{
		WINDOWPOS* winPos = (WINDOWPOS*)lParam;
		_printf("WM_WINDOWPOSCHANGED x %d y %d flag %u\n", winPos->x, winPos->y, winPos->flags);
		if(winPos->x != -32000 and (winPos->flags & SWP_NOMOVE) == 0)
		{
			_printf("save pos\n");
			window->setPosition(Vector2i(winPos->x, winPos->y));
		}
		if(winPos->x != -32000)
			window->setActivate(1);
		break;
	}

	case WM_SIZING:
	{
		/* fixed window size to minimum */
		Vector2i sizeMin = window->getSizeMin();
		if(sizeMin == 0) break;
		uint side = wParam;
		int* rc = (int*) lParam;
		Vector2i size(rc[2] - rc[0], rc[3] - rc[1]);
		if(size.x < sizeMin.x)
		{
			// left side
			if(side == 1 or side == 4 or side == 7)
				rc[0] = rc[2] - sizeMin.x;
			else
				rc[2] = rc[0] + sizeMin.x;
		}
		if(size.y < sizeMin.y)
		{
			// top side
			if(side >= 3 and side <= 5)
				rc[1] = rc[3] - sizeMin.y;
			else
				rc[3] = rc[1] + sizeMin.y;
		}
		break;
	}

	case WM_NOTIFY:
	{
		LPNMHDR notifyInfo = (LPNMHDR)lParam;
		int c = notifyInfo->code;
		switch(notifyInfo->code)
		{
		// Tooltips for toolbar
		case TTN_GETDISPINFO:
		{
			Toolbar* toolbar = (Toolbar*) window->findWidgetByType(WT_TOOLBAR);
			if(toolbar)
			{
				LPTOOLTIPTEXT lpt = (LPTOOLTIPTEXT)lParam;
				ToolbarButton* button = toolbar->findButtonById(lpt->hdr.idFrom);
				if(button)
					lpt->lpszText = button->mTooltip.p();
			}
			break;
		}
		// Combobox begin edit string
		case CBEN_BEGINEDIT:
		{
			_printf("CBEN BEGIN EDIT %u\n", notifyInfo->idFrom);
			Combobox* combobox = (Combobox*) window->findWidgetById(notifyInfo->idFrom);
			if(combobox)
				combobox->onEvent(0, 0);
			break;
		}
		// Combobox end edit string
		case CBEN_ENDEDIT:
		{
			Combobox* combobox = (Combobox*) window->findWidgetById(notifyInfo->idFrom);
			if(combobox)
			{
				PNMCBEENDEDIT editInfo = (PNMCBEENDEDIT) lParam;

				// press return; save selected item
				if(editInfo->iWhy == 2 and editInfo->iNewSelection != -1)
				{
					combobox->onEvent(1, editInfo->iNewSelection);
				}

				// press return, callback
				if(editInfo->iWhy == 2 and editInfo->fChanged != 0)
				{
					// combo id, selection, text
					uint_t endParams[3] = {notifyInfo->idFrom, editInfo->iNewSelection, (uint_t) editInfo->szText};
					pWidgetCallback(window, EVM_CB_ENDEDIT, (uint_t)endParams);
				}

				// return, escape; kill focus
				if(editInfo->iWhy != 4 and combobox->isFocused())
				{
					WT_SEND_MESSAGE(1, combobox);
					pWidgetCallback(window, EVM_CB_KILLFOCUS, (uint_t) notifyInfo->idFrom);
				}

				_printf("CBEN END EDIT %u why %u sel %d ch %d foc %d\ntext: %ls\n",
				        notifyInfo->idFrom, editInfo->iWhy, editInfo->iNewSelection,
				        editInfo->fChanged, combobox->isFocused(), editInfo->szText);
			}
			break;
		}

		// list view
		case LVN_BEGINLABELEDIT:
		{
			Listview* list = (Listview*) window->findWidgetById(notifyInfo->idFrom);
			list->setFocused(0);
			NMLVDISPINFO* pdi = (NMLVDISPINFO*) lParam;
			_printf("LVN_BEGINLABELEDIT text [%ls] item %d sub %d mask %u h %u\n",
			        pdi->item.pszText, pdi->item.iItem, pdi->item.iSubItem, pdi->item.mask, notifyInfo->idFrom);
			uint_t begParams[2] = {notifyInfo->idFrom, pdi->item.iItem};
			int ret = pWidgetCallback(window, EVM_LIST_BEGEDIT, (uint_t)begParams);
			if(!ret)
				return 1; // skip edit
			break;
		}
		case LVN_ENDLABELEDIT:
		{
			Listview* list = (Listview*) window->findWidgetById(notifyInfo->idFrom);
			list->setFocused(1);
			NMLVDISPINFO* pdi = (NMLVDISPINFO*) lParam;
			_printf("LVN_ENDLABELEDIT text [%ls] item %d sub %d mask %u h %u\n",
			        pdi->item.pszText, pdi->item.iItem, pdi->item.iSubItem, pdi->item.mask, notifyInfo->idFrom);
			uint_t endParams[3] = {notifyInfo->idFrom, pdi->item.iItem, (uint_t)pdi->item.pszText};
			pWidgetCallback(window, EVM_LIST_ENDEDIT, (uint_t)endParams);
			break;
		}
		// list view
		case LVN_KEYDOWN:
		{
			LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
			uint key = pnkd->wVKey;
			_printf("LVN_KEYDOWN %u\n", key);
			if(key == 38 or key == 40)
				pWidgetCallback(window, EVM_LIST_UPDATE, (uint_t)notifyInfo->idFrom);
			uint prm[2] = {notifyInfo->idFrom, key};
			pWidgetCallback(window, EVM_LIST_KEYDOWN, (uint_t)prm);
			break;
		}
		case LVN_COLUMNCLICK:
		{
			LPNMLISTVIEW listParam = (LPNMLISTVIEW) lParam;
			// iItem row | iSubItem column
			_printf("LVN_COLUMNCLICK item %d subitem %u win %u \n", listParam->iItem, listParam->iSubItem, window->getId());
			if(listParam->iItem == -1)
				pWidgetCallback(window, EVM_LIST_COLCLK, (uint_t)listParam->iSubItem);
		}
		case NM_CLICK:
		{
			pWidgetCallback(window, EVM_LIST_UPDATE, (uint_t)notifyInfo->idFrom);
			break;
		}
		case NM_DBLCLK:
		{
			pWidgetCallback(window, EVM_LIST_DBLCLK, (uint_t)notifyInfo->idFrom);
			break;
		}
		case NM_SETFOCUS:
		{
			Widget* list = window->findWidgetById(notifyInfo->idFrom);
			if(list)
			{
				list->setFocused(1);
				pWidgetCallback(window, EVM_LIST_SETFOCUS, (uint_t)notifyInfo->idFrom);
			}
			break;
		}
		case NM_KILLFOCUS:
		{
			Widget* list = window->findWidgetById(notifyInfo->idFrom);
			if(list)
			{
				list->setFocused(0);
				pWidgetCallback(window, EVM_LIST_KILLFOCUS, (uint_t)notifyInfo->idFrom);
			}
			break;
		}
		}
		break;
	}

	case WM_CONTEXTMENU:
	{
		pWidgetCallback(window, EVM_CONTEXTMENU, (uint_t)lParam);
		break;
	}
	} // main switch

	return DefWindowProc(hwnd, message, wParam, lParam);
}

bool SystemMessage()
{
	MSG msg = {0};
	while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GuiApplication::ptr()->onIdle();
	return (msg.message != WM_QUIT);
}

int _SendMessage(uint message, uint_t param)
{
	GuiApplication::ptr()->setMessage(message, param);
	return 1;
}

}