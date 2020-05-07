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
#include <Common/TextStream.h>
#include <Common/StringLib.h>
#include <Common/FileAccess.h>
#include <stdarg.h>
#include <stdio.h>

namespace tas
{

struct TextStream::impl
{
	FILE* filePtr;
	String name;
	char* line;
	char* buffer;
	uint len;
	bool apn;
	bool autoSave;
	bool console;
	bool readLine();
};

TextStream::TextStream(const String& fname, bool load, const String& mode, uint strLen)
{
	m = new impl;
	m->apn = 1;
	m->autoSave = 0;
	m->console = 0;
	m->len = 0;
	m->line = 0;
	m->filePtr = 0;
	m->buffer = 0;
	setStringLength(strLen);
	open(fname, load, mode);
}

TextStream::TextStream()
{
	m = new impl;
	m->apn = 1;
	m->len = 0;
	m->line = 0;
	m->autoSave = 0;
	m->filePtr = 0;
	m->console = 0;
	m->buffer = 0;
}

TextStream::~TextStream()
{
	close();
	safe_delete_array(m->line);
	safe_delete_array(m->buffer);
	safe_delete(m);
}

bool TextStream::open(const String& fname, bool load, const String& mode)
{
	if(state()) close();
	if(!m->line) setStringLength(512);
	m->name = fname;
	if(mode.length())
		m->filePtr = _wfopen(fname.p(), mode.p());
	else
		m->filePtr = _wfopen(fname.p(), load ? L"rb" : L"wb");
	return state();
}

bool TextStream::reopen(bool load, const String& mode)
{
	if(state()) close();
	if(!m->line) setStringLength(256);
	if(mode.length())
		m->filePtr = _wfopen(m->name.p(), mode.p());
	else
		m->filePtr = _wfopen(m->name.p(), load ? L"rb" : L"wb");
	m->apn = 1;
	return state();
}

void TextStream::close()
{
	if(m->filePtr)
		fclose(m->filePtr);
	m->filePtr = 0;
}

bool TextStream::state()
{
	return m->filePtr != 0;
}

bool TextStream::readLine()
{
	return m->readLine();
}

bool TextStream::impl::readLine()
{
	if(!feof(filePtr))
	{
		line[0] = 0;
		uint shift = 0;
		uint lenc = len;
		while(fgets(line + shift, lenc, filePtr))
		{
			int lens = stdlen(line);
			bool findn = 0;

			// find new line symbols
			if(lens > 0 and (line[lens-1] == 0x0D or line[lens-1] == 0x0A))
			{
				line[lens-1] = 0;
				findn = 1;
			}

			if(lens > 1 and line[lens-2] == 0x0D)
			{
				line[lens-2] = 0;
			}

			if(findn || feof(filePtr))
			{
				return 1;
			}

			line = (char*) stmrealloc(line, len, len * 2);
			len *= 2;
			shift = lens;
			lenc = len - lens;
		}
		if(shift)
			return 1;
	}
	return 0;
}

char* TextStream::getLine()
{
	return m->line;
}

void TextStream::writeLinef(char* format, ...)
{
	if(!state()) return;
	va_list args;
	va_start (args, format);
	/* #ifdef TAA_VSNPRINTF
		int size = vsnprintf(NULL, 0, format, args);
		char* buffer = new char[size+1];
	#else
		char buffer[256];
	#endif */
	if(!m->buffer)	m->buffer = new char[0x400];
	vsnprintf(m->buffer, 0x400, format, args);
	va_end(args);

	if(m->apn)
	{
		fprintf(m->filePtr, "%s\n", m->buffer);
		if(m->console) stdprintf("%s\n", m->buffer);
	}
	else
	{
		fprintf(m->filePtr, m->buffer);
		if(m->console) stdprintf(m->buffer);
	}
	if(m->autoSave) save();
	// safe_delete_array(buffer);
}

void TextStream::writeLine(char* s)
{
	if(!state()) return;
	if(m->apn)
	{
		fprintf(m->filePtr, "%s\n", s);
		if(m->console) stdprintf("%s\n", s);
	}
	else
	{
		fprintf(m->filePtr, s);
		if(m->console) stdprintf(s);
	}
	if(m->autoSave) save();
}

void TextStream::save()
{
	_wfreopen(m->name.p(), L"a", m->filePtr);
}

void TextStream::setStringLength(uint length)
{
	if(length <= m->len) return;
	m->line = (char*) stmrealloc(m->line, 0, length);
	m->len = length;
}

void TextStream::setAppendNewLine(bool state)
{
	m->apn = state;
}

void TextStream::setAutoSave(bool state)
{
	m->autoSave = state;
}

void TextStream::setConsoleOutput(bool state)
{
	m->console = state;
}

bool TextStream::getConsoleOutput()
{
	return m->console;
}

wint TextStream::getSize()
{
	return fileSize(m->name);
}

String TextStream::getName()
{
	return m->name;
}

}