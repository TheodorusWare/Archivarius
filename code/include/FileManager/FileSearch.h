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
#ifndef _TAH_FileSearch_h_
#define _TAH_FileSearch_h_

#include <Common/Config.h>
#include <Common/StringArray.h>

namespace tas
{

struct SearchFiles
{
	String buffer;
	uint bufferLen;
	byte hashEnable;
	uint hash;
	SearchFiles();
	~SearchFiles();
	int find(const String& directory, StringArray& files, StringArray& dirs);
	int countFiles(const String& directory); // with hash
};

}

#endif