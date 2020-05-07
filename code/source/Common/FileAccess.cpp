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
#include <Common/FileAccess.h>
#include <Common/StringLib.h>
#include <Common/UString.h>
#include <Common/Md5.h>
#include <Shellapi.h>
#include <time.h>

namespace tas
{

int findFilesImpl(const String& dir, const String& pattern, int recurse, StringArray& files, StringArray* dirs);

byte findFiles(const String& pattern, byte recurse, StringArray& files, StringArray* dirs)
{
	if(fileExist(pattern))
	{
		files.push_back(pattern);
		return 1;
	}
	String folder, mask;
	if(pattern.find(0x2A) != MAX_UINT32 or pattern.find(0x3F) != MAX_UINT32)
	{
		uint na = pattern.findr(0x5C); // '\'
		uint nb = pattern.findr(0x2F); // '/'
		if(na != MAX_UINT32 or nb != MAX_UINT32)
		{
			char c = 0x5C;
			if(na == MAX_UINT32 or (nb != MAX_UINT32 and na < nb))
				c = 0x2F;
			folder = pattern.substrx(c, 1, 0);
			mask = pattern.substrx(c, 1, 1);
		}
		else
		{
			folder = (char) 0x2E;
			mask = pattern;
		}
	}
	else
	{
		if(!folderExist(pattern))
			return 0;
		folder = pattern;
		mask = "";
	}
	mask.tolower();
	return findFilesImpl(folder, mask, recurse, files, dirs);
}

int findFilesImpl(const String& dir, const String& pattern, int recurse, StringArray& files, StringArray* dirs)
{
	byte ret = 1;
	HANDLE hFind = 0;
	WIN32_FIND_DATA data;

	String patName;
	String buf;
	buf.format("%ls\\*", dir.p());

	hFind = FindFirstFile(buf.p(), &data);

	if(hFind != INVALID_HANDLE_VALUE)
	{
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
				if(dirs or recurse)
				{
					patName.format("%ls\\%ls", dir.p(), data.cFileName);
					if(dirs)
						dirs->push_back(patName);
					if(recurse)
						findFilesImpl(patName, pattern, recurse, files, dirs);
				}
			}
			else // file
			{
				if(pattern.length())
				{
					patName = data.cFileName;
					patName.tolower();
					if(!stwpat(patName.p(), pattern.p()))
					{
						ret = FindNextFile(hFind, &data);
						continue;
					}
				}
				patName.format("%ls\\%ls", dir.p(), data.cFileName);
				files.push_back(patName);
			}

			ret = FindNextFile(hFind, &data);
		}
		FindClose(hFind);
	}
	else
		return 0;
	return 1;
}

void removeFolder(const String& dir)
{
	StringArray files, dirs;
	if(!findFilesImpl(dir, "", 1, files, &dirs))
		return;
	forn(files.size())
	DeleteFile(files[i].p());
	uint n = dirs.size();
	forn(dirs.size())
	RemoveDirectory(dirs[n-i-1].p());
	RemoveDirectory(dir.p());
}

bool removeFileShell(const String& file, bool recycleBin)
{
	String pszFrom;
	uint len = file.length();
	pszFrom.resize(len + 1);
	pszFrom = file;

	SHFILEOPSTRUCT fileop;
	fileop.hwnd   = NULL;        // no status display
	fileop.wFunc  = FO_DELETE;   // delete operation
	fileop.pFrom  = pszFrom.p(); // source file name as double null terminated string
	fileop.pTo    = NULL;        // no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;  // do not prompt the user
	if(recycleBin)
		fileop.fFlags |= FOF_ALLOWUNDO;
	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle     = NULL;
	fileop.hNameMappings         = NULL;

	int ret = SHFileOperation(&fileop);
	return (ret == 0);
}

bool removeFile(const String& file)
{
	return DeleteFile(file.p());
}

bool renameFile(const String& fileIn, const String& fileOut)
{
	return MoveFile(fileIn.p(), fileOut.p());
}

bool fileExist(const String& file)
{
	uint atr = GetFileAttributes(file.p());
	return (atr != INVALID_FILE_ATTRIBUTES and (atr & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

bool folderExist(const String& dir)
{
	uint atr = GetFileAttributes(dir.p());
	return (atr != INVALID_FILE_ATTRIBUTES and atr & FILE_ATTRIBUTE_DIRECTORY);
}

wint fileSize(const String& file)
{
	wint size = 0;
	WIN32_FIND_DATA data;
	HANDLE hFind = 0;
	hFind = FindFirstFile(file.p(), &data);
	if(hFind != INVALID_HANDLE_VALUE and (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		size = (wint)data.nFileSizeHigh << 32 | data.nFileSizeLow;
	}
	FindClose(hFind);
	return size;
}

// return file, folder name with case sensivity
bool fileName(const String& file, String& out)
{
	uint ret = 0;
	WIN32_FIND_DATA data;
	HANDLE hFind = 0;
	hFind = FindFirstFile(file.p(), &data);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		out = data.cFileName;
		ret = 1;
	}
	FindClose(hFind);
	return ret;
}

bool fileTime(const String& file, String& times, byte mode)
{
	uint size = 0;
	WIN32_FIND_DATA data;
	HANDLE hFind = 0;
	hFind = FindFirstFile(file.p(), &data);
	if(hFind == INVALID_HANDLE_VALUE)
		return 0;
	if(hFind)
	{
		SYSTEMTIME systime, systimeLocal;
		FILETIME* fileTime;
		switch(mode)
		{
		case 0:
			fileTime = &data.ftCreationTime;
			break;
		case 1:
			fileTime = &data.ftLastAccessTime;
			break;
		case 2:
			fileTime = &data.ftLastWriteTime;
			break;
		}
		int ret = FileTimeToSystemTime(fileTime, &systimeLocal);
		SystemTimeToTzSpecificLocalTime(NULL, &systimeLocal, &systime);
		if(!ret) return 0;
		times.format("%02u.%02u.%u %02u:%02u", systime.wDay, systime.wMonth,
		             systime.wYear, systime.wHour, systime.wMinute);
	}
	FindClose(hFind);
	return 1;
}

bool fileTimeValue(const String& file, uint* times, byte mode)
{
	uint size = 0;
	WIN32_FIND_DATA data;
	HANDLE hFind = 0;
	hFind = FindFirstFile(file.p(), &data);
	if(hFind == INVALID_HANDLE_VALUE)
		return 0;
	if(hFind)
	{
		SYSTEMTIME systime, systimeLocal;
		FILETIME* fileTime;
		switch(mode)
		{
		case 0:
			fileTime = &data.ftCreationTime;
			break;
		case 1:
			fileTime = &data.ftLastAccessTime;
			break;
		case 2:
			fileTime = &data.ftLastWriteTime;
			break;
		}
		times[0] = fileTime->dwLowDateTime;
		times[1] = fileTime->dwHighDateTime;
	}
	FindClose(hFind);
	return 1;
}

bool fileDosTimeValue(uint dosDate, uint* times)
{
	FILETIME fileTime;
	DosDateTimeToFileTime(dosDate >> 16, dosDate, &fileTime);
	times[0] = fileTime.dwLowDateTime;
	times[1] = fileTime.dwHighDateTime;
	return 1;
}

bool fileTimeString(uint* timei, String& times, byte mode)
{
	SYSTEMTIME systime, systimeLocal;
	FILETIME fileTime;
	fileTime.dwLowDateTime = timei[0];
	fileTime.dwHighDateTime = timei[1];
	int ret = FileTimeToSystemTime(&fileTime, &systimeLocal);
	if(mode == 1)
		SystemTimeToTzSpecificLocalTime(NULL, &systimeLocal, &systime);
	else
		systime = systimeLocal;
	if(!ret) return 0;
	times.format("%02u.%02u.%u %02u:%02u", systime.wDay, systime.wMonth,
	             systime.wYear, systime.wHour, systime.wMinute);
	return 1;
}

bool localTimeString(UString& times)
{
	SYSTEMTIME sm;
	GetLocalTime(&sm);
	times.format("%02u.%02u.%u %02u:%02u:%02u", sm.wDay, sm.wMonth,
	             sm.wYear, sm.wHour, sm.wMinute, sm.wSecond);
	return 1;
}

bool localTimeMd5String(UString& times)
{
	localTimeString(times);
	Md5 mdFive;
	byte* mdMessage = mdFive.calculate(reinterpret_cast<byte*>(times.p()), times.length(), 1);
	times.assign(reinterpret_cast<char*>(mdMessage), 16);
	times.tohex();
	return 1;
}

// create dir, /a/b/c >> a/b
void crdir(const String& si)
{
	wchar_t* q = si.p();
	for(wchar_t* s = q; *++s;)
	{
		if(*s == 0x5C or *s == 0x2F)
		{
			*s = 0;
			CreateDirectory(q, 0);
			*s = 0x5C;
		}
	}
}

void createFolder(const String& dir, byte mode)
{
	if(!dir.length() or folderExist(dir)) return;
	crdir(dir);
	if(mode) CreateDirectory(dir.p(), 0);
}

void hideFile(const String& file, byte mode)
{
	int attr = GetFileAttributes(file.p());
	if(mode) attr |= FILE_ATTRIBUTE_HIDDEN;
	else attr &= ~FILE_ATTRIBUTE_HIDDEN;
	SetFileAttributes(file.p(), attr);
}

bool generateFileName(String& buffer, const String& base, byte mode, byte num, byte root)
{
	String windir;
	UString timehexu;
	String timehex;
	timehexu.reserve(64);
	timehex.reserve(64);
	if(root)
	{
		windir.reserve(1024);
		GetTempPath(1024, windir.p());
	}
	while(1)
	{
		localTimeMd5String(timehexu);
		timehexu.resize(num);
		timehexu.tolower();
		timehex.assign(timehexu.p(), timehexu.length());
		if(root)
			buffer.format("%ls%ls%ls", windir.p(), base.p(), timehex.p());
		else
			buffer.format("%ls%ls", base.p(), timehex.p());
		if(!mode and !fileExist(buffer))
			break;
		if(mode and !folderExist(buffer))
			break;
	}
	buffer.update();
	return 1;
}

}