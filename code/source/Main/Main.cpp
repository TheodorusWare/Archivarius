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
#include <Common/Md5.h>

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#include <shellapi.h>
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#include <unistd.h>
#include <sys/time.h>
#endif

using namespace tas;

ArchiveArg* args = 0;

void help();
void parserCmdLine(int argc, wchar_t* argv[]);
void appendFilesToArchive();
void extractFilesFromArchive();
void deleteFilesFromArchive();
void renameFilesInArchive();
void extractFileList();
void findModule();
void parseError();

struct ArchiverVar
{
	String errorString;
	byte errorNum;
	byte errorNumArc;
	byte mode;
	ArchiverVar()
	{
		errorNum = 0;
		errorNumArc = 0;
		mode = -1;
	}
	~ArchiverVar()
	{
	}
};

static ArchiverVar* arcv = 0;

// console
int main_console(int argc, wchar_t* argv[])
{
	arcv = new ArchiverVar;

	if(argc < 3)
	{
		arcv->errorNum = 1;
		return 0;
	}

	byte savep = 1;

	if(argv[1][0] == 'a')
		arcv->mode = 0;
	else if(argv[1][0] == 'e')
		arcv->mode = 1;
	else if(argv[1][0] == 'x')
	{
		arcv->mode = 1;
		savep = 0;
	}
	else if(argv[1][0] == 'l')
		arcv->mode = 2;
	else if(argv[1][0] == 'd')
		arcv->mode = 3;
	else if(argv[1][0] == 'r')
		arcv->mode = 4;

	if(arcv->mode > 4)
	{
		arcv->errorNum = 2;
		return 0;
	}

	if(arcv->mode and !fileExist(argv[2]))
	{
		arcv->errorNum = 3;
		return 0;
	}

	args = new ArchiveArg;
	args->mode = arcv->mode;
	args->savePath = savep;
	args->archiveName = argv[2];

	parserCmdLine(argc, argv);
	findModule();

	if(!arcv->mode and !args->files.size())
	{
		arcv->errorNum = 4;
		return 0;
	}

	if(!arcv->mode and args->encodeMethod > 2)
	{
		arcv->errorNum = 5;
		return 0;
	}

	if(!arcv->mode and args->encodeLevel > 9)
	{
		arcv->errorNum = 6;
		return 0;
	}

	if(arcv->mode == 0)
		appendFilesToArchive();
	else if(arcv->mode == 1)
		extractFilesFromArchive();
	else if(arcv->mode == 2)
		extractFileList();
	else if(arcv->mode == 3)
		deleteFilesFromArchive();
	else if(arcv->mode == 4)
		renameFilesInArchive();

	if(!arcv->errorNum and arcv->mode != 2)
		stdprintf("\n\nComplete\n\n");

	return 0;
}

// console version
int wmain(int argc, wchar_t* argv[])
{
	allocatorSetClientMemoryRoutines();
	allocatorInit();
	// allocatorSetTableMode(0x40); // memory leak table
	allocatorSetMemorySize(1 * MB, 64 * KB); // 1

	byte err = 0;

	// wchar_t** argw = CommandLineToArgvW(GetCommandLineW(), &argc);
	main_console(argc, argv);

	parseError();

	err = arcv->errorNum;

	safe_delete(args);
	safe_delete(arcv);

	if(!err) allocatorInfo();
	allocatorClean();

	return 0;
}

void parseError()
{
	if(arcv->errorNum == 1)
	{
		help();
		return;
	}
	if(arcv->errorNum)
	{
		if(arcv->errorNum < 7)
			help();

		char splitLine[41] = {0};
		forn(40) splitLine[i] = 0xC4;
		stdprintf("\n%s\n\n", splitLine);

		if(arcv->mode < 2)
			stdprintf("%s ", !arcv->mode ? "Appending" : "Unpacking");

		if(arcv->errorNum == 7)
			stdprintf("error %u.%u\n\n", arcv->errorNum, arcv->errorNumArc);
		else
			stdprintf("error %u\n\n", arcv->errorNum);

		if(arcv->errorNum == 1)
			stdprintf("Small command line arguments \n");
		if(arcv->errorNum == 2)
			stdprintf("Wrong command \n");
		if(arcv->errorNum == 3)
			stdprintf("Archive not exist \n");
		if(arcv->errorNum == 4)
			stdprintf("No appending files \n");
		if(arcv->errorNum == 5)
			stdprintf("Wrong method \n");
		if(arcv->errorNum == 6)
			stdprintf("Wrong level \n");
		if(arcv->errorNum == 7)
			stdprintf("%ls\n", arcv->errorString.p());

		stdprintf("\n%s\n", splitLine);
	}
}

void appendFilesToArchive()
{
	assert(args->logName.length());
	assert(args->archiveName.length());

	int ret = 0;
	Archive archive;
	if(!archive.createLog(args->logName))
		return;
	if(fileExist(args->archiveName))
		ret = archive.open(args->archiveName);

	if(!ret and !archive.create(args->archiveName))
	{
		stdprintf("error %ls\n", args->archiveName.p());
		return;
	}

	if(args->encodeMethod)
	{
		archive.setValue(args->encodeMethod, 0);
		archive.setValue(args->encodeLevel, 1);
		archive.setValue(args->encodeThreads, 2);
		archive.setValue(args->encodeDictSize, 3);
		archive.setValue(args->encodeWordSize, 4);
		archive.setValue(args->encodeMatchCycle, 5);
		archive.setValue(args->encodeMatchMin, 9);
		archive.setValue(args->encodeMatchRolz, 20);
		archive.setValue(args->encodeCmix, 21);
		archive.setValue(args->encodeSuffix, 15);
		archive.setValue(args->encodeBigraph, 16);
	}

	if(args->cryptStr.length())
	{
		archive.setValue((uint_t) args->cryptStr.p(), 6);
		if(!args->cryptState)
			args->cryptState = 2;
		archive.setValue((uint_t) args->cryptState, 19);
	}

	if(args->eventCallback)
		archive.setValue((uint_t)args->eventCallback, 8);

	archive.setValue(args->cutPath, 17);
	if(!archive.append(args->files))
	{
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		arcv->errorString = archive.getErrorStr();
		return;
	}
}

void extractFilesFromArchive()
{
	int ret = 0;
	Archive archive;

	if(args->cryptStr.length())
		archive.setValue((uint_t) args->cryptStr.p(), 6);

	if(!archive.createLog(args->logName))
		return;

	ret = archive.open(args->archiveName, 1);

	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}

	archive.setValue((uint_t) args->dir.p(), 10);
	archive.setValue((uint_t) args->savePath, 11);

	if(!archive.extract(&args->files))
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}
}

void deleteFilesFromArchive()
{
	int ret = 0;
	Archive archive;

	if(args->cryptStr.length())
		archive.setValue((uint_t) args->cryptStr.p(), 6);

	if(!archive.createLog(args->logName))
		return;

	ret = archive.open(args->archiveName, 1);

	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}

	ret = archive.remove(args->files);
	if(!ret and archive.getError() == 4)
	{
		archive.close();
		removeFile(args->archiveName);
		ret = 1;
	}

	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}
}

void renameFilesInArchive()
{
	int ret = 0;
	Archive archive;

	if(args->cryptStr.length())
		archive.setValue((uint_t) args->cryptStr.p(), 6);

	if(!archive.createLog(args->logName))
		return;

	ret = archive.open(args->archiveName, 1);

	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}

	ret = archive.rename(args->files);
	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}
}

void extractFileList()
{
	Archive archive;
	if(args->cryptStr.length())
		archive.setValue((uint_t) args->cryptStr.p(), 6);
	if(!archive.createLog(args->logName))
		return;

	int ret = archive.open(args->archiveName, 1);

	if(!ret)
	{
		arcv->errorString = archive.getErrorStr();
		arcv->errorNum = 7;
		arcv->errorNumArc = archive.getError();
		return;
	}

	if(!archive.list())
		return;
}

void parserCmdLine(int argc, wchar_t* argv[])
{
	String pattern;
	String arg;
	StringArray list;
	arg.reserve(512);

#define CMP(s,n) (arg.compare(L##s, n) == 0)

	forn(argc - 3)
	{
		arg = argv[i + 3];

		if(CMP("-p", 2))
			args->cryptStr = arg.substr(2);

		else if(CMP("-ml", 3))
			args->encodeLevel = stdwtoi(arg.p() + 3);

		else if(CMP("-mt", 3))
			args->encodeThreads = stdwtoi(arg.p() + 3);

		else if(CMP("-mw", 3))
			args->encodeWordSize = stdwtoi(arg.p() + 3);

		else if(CMP("-mc", 3))
			args->encodeMatchCycle = stdwtoi(arg.p() + 3);

		else if(CMP("-mn", 3))
			args->encodeMatchMin = stdwtoi(arg.p() + 3);

		else if(CMP("-ms", 3))
			args->encodeSuffix = stdwtoi(arg.p() + 3);

		else if(CMP("-mr", 3))
			args->encodeMatchRolz = stdwtoi(arg.p() + 3);

		else if(CMP("-mx", 3))
			args->encodeCmix = 1;

		else if(CMP("-bg", 3))
			args->encodeBigraph = 1;

		else if(CMP("-cr", 3))
			args->cryptState = stdwtoi(arg.p() + 3);

		else if(CMP("-md", 3))
		{
			// parse size suffixes k, m
			// for setting size in kilo, mega bytes
			String dictsz = arg.substr(3);
			byte len = dictsz.length();
			if(dictsz[len - 1] == 'k')
			{
				dictsz[len - 1] = 0;
				args->encodeDictSize = stdwtoi(dictsz.p()) * KB;
			}
			else if(dictsz[len - 1] == 'm')
			{
				dictsz[len - 1] = 0;
				args->encodeDictSize = stdwtoi(dictsz.p()) * MB;
			}
			else // 2^n bytes
				args->encodeDictSize = 1 << stdwtoi(dictsz.p());
		}
		else if(CMP("-m", 2))
		{
			args->encodeMethod = stdwtoi(arg.p() + 2);
		}
		else if(CMP("-l", 2))
		{
			args->listFiles = arg.substr(2);
		}
		else if(arg[0] != '-')
		{
			uint len = arg.length();
			if(args->mode < 2 and i + 4 == argc and arg[len-1] == PATH_SEP)
			{
				// work dir
				arg[len-1] = 0;
				arg.update();
				args->dir = arg;
				continue;
			}
			if(!args->mode) // append files
			{
				list.push_back(arg);
			}
			else // extract files
			{
				args->files.push_back(arg);
			}
		}
	}

#undef CMP

	if(args->dir.length())
		args->cutPath = args->dir.length() + 1;

	if(args->listFiles.length())
	{
		TextStream listStream(args->listFiles, 1, "", 0x400);
		if(!listStream.state())
			stdprintf("file list %ls not exist \n", args->listFiles.p());
		else
		{
			String line;
			while(listStream.readLine())
			{
				line = listStream.getLine();
				if(line.length())
				{
					if(!args->mode)
						list.push_back(line);
					else
						args->files.push_back(line);
				}
			}
		}
	}

	if(!args->mode)
	{
		forn(list.size())
		{
			if(args->dir.length())
			{
				pattern.format("%ls%c%ls", args->dir.p(), PATH_SEP, list[i].p());
				findFiles(pattern, 1, args->files);
			}
			else
			{
				findFiles(list[i], 1, args->files);
			}
		}
		if(!list.size() and args->dir.length())
		{
			findFiles(args->dir, 1, args->files);
		}
	}
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
	mol.resize(bs + 1);
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

	tol.format("%lsNotification%cAllocator.log", mol.p(), PATH_SEP);
	allocatorSetLog(tol.p());

	tol.format("%lsNotification%cArchivarius.log", mol.p(), PATH_SEP);
	args->logName = tol;

	tol.format("%lsNotification", mol.p());
	createFolder(tol, 1);
	return;
}

void help()
{
	stdprintf("\nArchivarius %u.%u.%u\n\n", TAA_VERSION_MAJOR, TAA_VERSION_MINOR, TAA_VERSION_PATCH);
	stdprintf
	(
	    "Command line syntax\n\n"

	    "  archivarius <command> <archive> [-keys] [files ..] [path\\]\n\n"

	    "Commands\n\n"

	    "  a  Append files to archive\n"
	    "  e  Extract files from archive with full paths\n"
	    "  x  Extract files from archive without paths\n"
	    "  l  List archive contents\n\n"

	    "Keys\n\n"

	    "  -m<n>   Compression method\n"
	    "  -ml<n>  Compression level\n"
	    "  -md<n>  Dictionary size\n"
	    "  -mw<n>  Maximum match length\n"
	    "  -mn<n>  Minimum match length\n"
	    "  -mt<n>  Compression threads\n"
	    "  -mc<n>  Cycles count\n\n"

	    "For detailed information see readme.txt\n\n"

	    "Copyright (C) 2018-2020 Theodorus Software\n"
	);
}