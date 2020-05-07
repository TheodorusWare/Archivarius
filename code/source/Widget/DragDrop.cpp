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
#include <Shellapi.h>
#include <Widget/DragDrop.h>

namespace tas
{

DragDrop::DragDrop()
{
	mHandle = 0;
}

DragDrop::~DragDrop()
{
}

bool DragDrop::setHandle(void* handle)
{
	mHandle = handle;
	return 1;
}

uint DragDrop::getCount()
{
	if(!mHandle) return 0;
	return DragQueryFile((HDROP)mHandle, -1, 0, 0);
}

bool DragDrop::getItem(uint item, String& buffer)
{
	if(!mHandle) return 0;
	DragQueryFile((HDROP)mHandle, item, buffer.p(), buffer.capacity());
	buffer.update();
	return 1;
}

}