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
#include <Common/StringArray.h>
#include <Common/StringLib.h>
#include <Common/Allocator.h>
#include <Compress/Archive.h>
#include <Common/FileAccess.h>
#include <Common/TextStream.h>
#include "ArchiveArg.h"

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#include <FileManager/FileManagerDef.h>
#include <FileManager/FileManager.h>
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#include <unistd.h>
#include <sys/time.h>
#endif

using namespace tas;

ArchiveArg* args = 0;

void findModule();
String parserCmdLine(String srcCmdLine);

#ifdef TAA_ARCHIVARIUS_GUI_NATIVE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, wchar_t* lpCmdLine, int nShowCmd)
#else
int wmain(int argc, wchar_t* argv[])
#endif
{
	allocatorSetClientMemoryRoutines();
	allocatorInit();
	// allocatorSetTableMode(0x40); // memory leak table
	allocatorSetMemorySize(1 * MB, 64 * KB);

	args = new ArchiveArg;
	findModule();

	{
		FileManager fileManager;
#ifdef TAA_ARCHIVARIUS_GUI_NATIVE
		if(stwlen(lpCmdLine))
		{
			String cmdLine = parserCmdLine(lpCmdLine);
			fileManager.create(cmdLine);
		}
		else
			fileManager.create(L"");
#else
		fileManager.create(argc > 1 ? argv[1] : 0);
#endif
	}

	createFolder(args->logDirName, 1);
	safe_delete(args);
	allocatorInfo();
	allocatorClean();
	return 0;
}

String parserCmdLine(String srcCmdLine)
{
	// trim quotes
	String dstCmdLine = srcCmdLine;
	int quote = dstCmdLine.find(0x22);
	if(quote != -1)
	{
		dstCmdLine.substrs(quote + 1);
		dstCmdLine.trim(0x22, 1);
	}
	return dstCmdLine;
}

void findModule()
{
	// full module path with '\' end
	String mol;
	String tol;
	mol.reserve(0x400);
	tol.reserve(0x400);

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	GetModuleFileName(0, mol.p(), 0x400);
	mol.update();
	int bs = mol.findr(PATH_SEP);
	assert(bs != -1);
	mol.resize(bs+1);
	args->module = mol;
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#if TAA_PLATFORM_TYPE == TAA_PLATFORM_MOBILE
	mol = "/sdcard/android/data/com.archiver/";
#else
	readlink("/proc/self/exe", mol.p(), 0x400);
	int bs = mol.findr(PATH_SEP);
	assert(bs != -1);
	mol.resize(bs+1);
#endif
#endif

	tol.format("%sNotification%cAllocator.log", mol.p(), PATH_SEP);
	allocatorSetLog(tol.p());

	tol.format("%sNotification%cArchivarius.log", mol.p(), PATH_SEP);
	args->logName = tol;

	tol.format("%sNotification", mol.p());
	args->logDirName = tol;
	createFolder(tol, 1);
}

namespace tas
{

String& getLogName()
{
	return args->logName;
}

}