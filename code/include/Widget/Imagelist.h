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
#ifndef _TAH_GuiImagelist_h_
#define _TAH_GuiImagelist_h_

#include <Common/Config.h>
#include <Common/String.h>
#include <Common/Vector2i.h>

namespace tas
{

class Imagelist
{
	HIMAGELIST mHandle;
	Vector2i mSize;
public:
	Imagelist();
	~Imagelist();
	HIMAGELIST getHandle();
	bool create(Vector2i size, uint count);
	bool destroy();
	bool appendIcon(HICON icon);
	bool loadIcon(HINSTANCE module, uint resourceId);
	bool loadIcon(const String& module, uint resourceId);
	bool loadIcon(uint resourceId);
	bool loadFileIcon(const String& file, uint type);

	static void* loadImage(uint resourceId);
	static void* loadImageIcon(uint resourceId, Vector2i size);
	// module : .exe, .dll
};

}

#endif