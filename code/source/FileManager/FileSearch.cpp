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
#include <FileManager/FileManagerDef.h>
#include <FileManager/FileSearch.h>
#include <Common/StringLib.h>
#include <Common/Hash.h>

namespace tas
{

SearchFiles::SearchFiles()
{
	bufferLen = 0;
}

SearchFiles::~SearchFiles()
{
	bufferLen = 0;
	hashEnable = 0;
	hash = 0;
}

int SearchFiles::find(const String& directory, StringArray& files, StringArray& dirs)
{
	hash = 0;
	int ret = 1;
	HANDLE hFind = 0;
	WIN32_FIND_DATA data;
	uint dirLen = directory.length();
	if(bufferLen == 0 or bufferLen < dirLen + 10)
	{
		if(bufferLen == 0)
			bufferLen = 1024;
		while(bufferLen < dirLen + 10)
			bufferLen *= 2;
		buffer.reserve(bufferLen);
	}
	buffer.format("%ls\\*", directory.p());

	hFind = FindFirstFile(buffer.p(), &data);

	if(hFind == INVALID_HANDLE_VALUE)
		return 0;

	while(ret)
	{
		bool skip = 0;
		uint slen = stwlen(data.cFileName);
		if(slen == 1 and data.cFileName[0] == 0x2E) // '.'
			skip = 1;
		if(slen == 2 and data.cFileName[0] == 0x2E and data.cFileName[1] == 0x2E)
			skip = 1;
		if(skip)
		{
			ret = FindNextFile(hFind, &data);
			continue;
		}
		// directory
		if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			dirs.push_back(data.cFileName);
		}
		else // file
		{
			files.push_back(data.cFileName);
		}
		if(hashEnable)
			hash = HashLy((byte*)data.cFileName, stwlen(data.cFileName) * 2, hash);
		ret = FindNextFile(hFind, &data);
	}
	FindClose(hFind);
	return 1;
}

int SearchFiles::countFiles(const String& directory)
{
	hash = 0;
	int count = 0;
	int ret = 1;
	HANDLE hFind = 0;
	WIN32_FIND_DATA data;
	uint dirLen = directory.length();
	if(bufferLen == 0 or bufferLen < dirLen + 10)
	{
		if(bufferLen == 0)
			bufferLen = 1024;
		while(bufferLen < dirLen + 10)
			bufferLen *= 2;
		buffer.reserve(bufferLen);
	}
	buffer.format("%ls\\*", directory.p());
	hFind = FindFirstFile(buffer.p(), &data);

	if(hFind == INVALID_HANDLE_VALUE)
		return 0;

	while(ret)
	{
		bool skip = 0;
		uint slen = stwlen(data.cFileName);
		if(slen == 1 and data.cFileName[0] == 0x2E)
			skip = 1;
		if(slen == 2 and data.cFileName[0] == 0x2E and data.cFileName[1] == 0x2E)
			skip = 1;
		if(skip)
		{
			ret = FindNextFile(hFind, &data);
			continue;
		}
		if(hashEnable)
			hash = HashLy((byte*)data.cFileName, stwlen(data.cFileName) * 2, hash);
		count++;
		ret = FindNextFile(hFind, &data);
	}
	FindClose(hFind);
	return count;
}

}