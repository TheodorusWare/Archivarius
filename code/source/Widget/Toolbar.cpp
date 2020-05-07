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
#include <Widget/Window.h>
#include <Widget/Toolbar.h>
#include <Widget/Imagelist.h>

namespace tas
{

ToolbarButton::ToolbarButton()
{
	mId = 0;
	mIconId = 0;
}

ToolbarButton::ToolbarButton(uint id, uint iconId, const String& caption, const String& tooltip)
{
	mId = id;
	mIconId = iconId;
	mCaption = caption;
	mTooltip = tooltip;
}

ToolbarButton::~ToolbarButton()
{
}

Toolbar::Toolbar()
{
	mType = WT_TOOLBAR;
	mTooltipStyle = 0;
	mImagelist = 0;
}

Toolbar::Toolbar(Window* parent, uint id, Vector2i buttonSize, bool tooltipStyle, Imagelist* imageList)
{
	mType = WT_TOOLBAR;
	mImagelist = imageList;
	mTooltipStyle = tooltipStyle;
	mParent = parent;
	mButtonSize = buttonSize;
	parent->registerWidget(this);
	setId(id);
}

Toolbar::~Toolbar()
{
	mButtons.clear_ptr();
}

bool Toolbar::create()
{
	uint wstyle = WS_CHILD | WS_VISIBLE;
	if(mTooltipStyle)
		wstyle |= TBSTYLE_TOOLTIPS;
	createWindow(TOOLBARCLASSNAME, wstyle);
	createButtons();
	return 1;
}

void Toolbar::appendButton(ToolbarButton* button)
{
	mButtons.push_back(button);
}

void Toolbar::createButtons()
{
	int ImagelistID = 0;

	TBBUTTON* tbButtons = new TBBUTTON[mButtons.size()];
	menset(tbButtons, 0, sizeof(TBBUTTON) * mButtons.size());
	forn(mButtons.size())
	{
		tbButtons[i].iBitmap = MAKELONG(mButtons[i]->mIconId, ImagelistID);
		tbButtons[i].idCommand = mButtons[i]->mId;
		tbButtons[i].fsState = TBSTATE_ENABLED;
		tbButtons[i].fsStyle = 0;
		tbButtons[i].iString = (INT_PTR)mButtons[i]->mCaption.p();
	}

	// Set the image list.
	SendMessage(mHandle, TB_SETIMAGELIST, (WPARAM)ImagelistID, (LPARAM)mImagelist->getHandle());

	SendMessage(mHandle, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(mHandle, TB_ADDBUTTONS, (WPARAM)mButtons.size(), (LPARAM)tbButtons);

	// Set buttons size
	if(mButtonSize != 0)
		SendMessage(mHandle, TB_SETBUTTONSIZE, 0, MAKELPARAM(mButtonSize.x, mButtonSize.y));

	// Resize the toolbar, and then show it.
	SendMessage(mHandle, TB_AUTOSIZE, 0, 0);

	safe_delete_array(tbButtons);
}

ToolbarButton* Toolbar::findButtonById(uint id)
{
	forn(mButtons.size())
	{
		if(mButtons[i]->mId == id)
			return mButtons[i];
	}
	return 0;
}

void Toolbar::onSize()
{
	move(Vector2i(), Vector2i());
}

}