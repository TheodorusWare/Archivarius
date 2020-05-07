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
#include <CommCtrl.h>
#include <Shellapi.h>
#include <shlobj.h>
#include <Widget/Imagelist.h>

namespace tas
{

Imagelist::Imagelist()
{
	mHandle = 0;
}

Imagelist::~Imagelist()
{
	if(mHandle)
		ImageList_Destroy(mHandle);
}

HIMAGELIST Imagelist::getHandle()
{
	return mHandle;
}

bool Imagelist::create(Vector2i size, uint count)
{
	mSize = size;
	mHandle = ImageList_Create(size.x, size.y, ILC_COLOR32, count, 1);
	return 1;
}

bool Imagelist::destroy()
{
	if(mHandle)
	{
		ImageList_Destroy(mHandle);
		mHandle = 0;
	}
	return 1;
}

bool Imagelist::appendIcon(HICON icon)
{
	if(!mHandle) return 0;
	ImageList_AddIcon(mHandle, icon);
	return 1;
}

bool Imagelist::loadIcon(uint resourceId)
{
	return loadIcon(GetModuleHandle(0), resourceId);
}

void* Imagelist::loadImage(uint resourceId)
{
	return (HBITMAP) LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), IMAGE_BITMAP, 0, 0, LR_SHARED);
}

void* Imagelist::loadImageIcon(uint resourceId, Vector2i size)
{
	return (HICON) LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(resourceId), IMAGE_ICON, size.x, size.y, LR_SHARED);
}

bool Imagelist::loadIcon(HINSTANCE module, uint resourceId)
{
	if(!module)	module = GetModuleHandle(0);
	if(!module or !mHandle) return 0;
	HICON icon = (HICON)LoadImage(module, MAKEINTRESOURCE(resourceId), IMAGE_ICON, mSize.x, mSize.y, LR_SHARED);
	if(!icon) return 0;
	ImageList_AddIcon(mHandle, icon);
	return 1;
}

bool Imagelist::loadIcon(const String& module, uint resourceId)
{
	if(!module.length() or !mHandle) return 0;
	HINSTANCE moduleHandle = LoadLibrary(module.p());
	if(!moduleHandle)	return 0;
	bool ret = loadIcon(moduleHandle, resourceId);
	FreeLibrary(moduleHandle);
	return ret;
}

bool Imagelist::loadFileIcon(const String& file, uint type)
{
	SHFILEINFO sfi;
	menset(&sfi, 0, sizeof(sfi));
	uint attribute = !type ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY;
	uint flag = SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
	DWORD_PTR ret = SHGetFileInfo(file.p(), attribute, &sfi, sizeof(sfi), flag);
	if(ret) ImageList_AddIcon(mHandle, sfi.hIcon);
	return (ret != 0);
}

}