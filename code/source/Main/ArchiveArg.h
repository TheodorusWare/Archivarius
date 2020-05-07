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
#ifndef _TAH_ArchiveArg_h_
#define _TAH_ArchiveArg_h_

#include <Common/Config.h>
#include <Common/StringArray.h>

namespace tas
{

struct ArchiveArg
{
	StringArray files;
	uint cutPath;
	uint savePath;
	String archiveName;
	String dir;
	String module;
	String logName;
	String logDirName;
	byte  cryptState;
	String cryptStr;
	String listFiles;
	void* eventCallback; // function pointer
	byte mode;

	byte encodeMethod;
	byte encodeLevel;
	byte encodeThreads;
	uint encodeDictSize;
	uint encodeWordSize;
	half encodeMatchCycle;
	byte encodeMatchMin;
	byte encodeMatchRolz;
	byte encodeSuffix;
	byte encodeBigraph;
	byte encodeCmix;

	ArchiveArg()
	{
		mode = 0;
		cryptState = 0;
		eventCallback = 0;
		cutPath = 0;
		savePath = 1;
		encodeMethod = 0;
		encodeLevel = 6;
		encodeThreads = 0;
		encodeDictSize = 0;
		encodeWordSize = 0;
		encodeMatchCycle = 0;
		encodeMatchMin = 0;
		encodeMatchRolz = 0;
		encodeSuffix = 0;
		encodeBigraph = 0;
		encodeCmix = 0;
	}
	~ArchiveArg()
	{
	}
};

}

#endif