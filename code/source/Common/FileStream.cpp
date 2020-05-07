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
#include <Common/FileStream.h>
#include <Common/StringLib.h>
#include <Common/FileAccess.h>
#include <stdio.h>

namespace tas
{

struct FileStream::impl
{
	FILE* filePtr;
};

FileStream::FileStream(const String& _fname, bool load, const String& mode)
{
	m = new impl;
	fname = _fname;
	m->filePtr = 0;
	rwb = 0;
	closf = 1;
	open(fname, load, mode);
}

FileStream::FileStream()
{
	m = new impl;
	m->filePtr = 0;
	rwb = 0;
	closf = 0;
}

FileStream::~FileStream()
{
	close();
	safe_delete(m);
}

bool FileStream::open(const String& _fname, bool load, const String& mode)
{
	fname = _fname;
	close();
	if(mode.length())
		m->filePtr = _wfopen(fname.p(), mode.p());
	else
		m->filePtr = _wfopen(fname.p(), load ? L"rb" : L"wb");
	closf = 1;
	return (m->filePtr != 0);
}

void FileStream::setf(void* fl, bool close)
{
	m->filePtr = (FILE*) fl;
	closf = close;
}

void* FileStream::getf()
{
	return m->filePtr;
}

String FileStream::getName()
{
	return fname;
}

void FileStream::close()
{
	if(m->filePtr && closf)
	{
		fclose(m->filePtr);
		m->filePtr = 0;
	}
}

bool FileStream::state()
{
	return m->filePtr != 0;
}

wint FileStream::size()
{
	return fileSize(fname);
}

void FileStream::begin()
{
	rewind(m->filePtr);
}

bool FileStream::seek(int offset, int origin)
{
	return fseek(m->filePtr, offset, origin) == 0;
}

bool FileStream::seekw(int64 offset, int origin)
{
#if TAA_COMPILER == TAA_COMPILER_MSVC
	return _fseeki64(m->filePtr, offset, origin) == 0;
#else
	// return fseeko64(m->filePtr, offset, origin) == 0;
	return 0;
#endif
}

int FileStream::tell()
{
	return ftell(m->filePtr);
}

wint FileStream::tellw()
{
#if TAA_COMPILER == TAA_COMPILER_MSVC
	return _ftelli64(m->filePtr);
#else
	// return ftello64(m->filePtr);
	return 0;
#endif
}

uint8 FileStream::readByte()
{
	uint8 v = 0;
	rwb = fread(&v, sizeof(uint8), 1, m->filePtr);
	return v;
}

half FileStream::readHalf()
{
	half v = 0;
	rwb = fread(&v, sizeof(half), 1, m->filePtr);
	return v;
}

uint FileStream::readUint()
{
	uint v = 0;
	rwb = fread(&v, sizeof(uint), 1, m->filePtr);
	return v;
}

wint FileStream::readWint()
{
	wint v = 0;
	rwb = fread(&v, sizeof(wint), 1, m->filePtr);
	return v;
}

int FileStream::readInt()
{
	int v = 0;
	rwb = fread(&v, sizeof(int), 1, m->filePtr);
	return v;
}

float FileStream::readFloat()
{
	float v = 0;
	rwb = fread(&v, sizeof(float), 1, m->filePtr);
	return v;
}

double FileStream::readDouble()
{
	double v = 0;
	rwb = fread(&v, sizeof(double), 1, m->filePtr);
	return v;
}

uint FileStream::readBuffer(void* buffer, uint size)
{
	return rwb = fread(buffer, 1, size, m->filePtr);
}

void FileStream::storeByte(uint8 v)
{
	rwb = fwrite(&v, sizeof(uint8), 1, m->filePtr);
}

void FileStream::storeHalf(half v)
{
	rwb = fwrite(&v, sizeof(half), 1, m->filePtr);
}

void FileStream::storeUint(uint v)
{
	rwb = fwrite(&v, sizeof(uint), 1, m->filePtr);
}

void FileStream::storeWint(wint v)
{
	rwb = fwrite(&v, sizeof(wint), 1, m->filePtr);
}

void FileStream::storeInt(int v)
{
	rwb = fwrite(&v, sizeof(int), 1, m->filePtr);
}

void FileStream::storeFloat(float v)
{
	rwb = fwrite(&v, sizeof(float), 1, m->filePtr);
}

void FileStream::storeDouble(double v)
{
	rwb = fwrite(&v, sizeof(double), 1, m->filePtr);
}

uint FileStream::storeBuffer(void* buffer, uint size)
{
	return rwb = fwrite(buffer, 1, size, m->filePtr);
}

bool FileStream::eof()
{
	return feof(m->filePtr);
}

void FileStream::save()
{
	_wfreopen(fname.p(), L"ab", m->filePtr);
}

uint FileStream::grwb()
{
	return rwb;
}

void FileStream::setbuf(char* buf, int mode, uint size)
{
	setvbuf(m->filePtr, buf, mode, size);
}

}