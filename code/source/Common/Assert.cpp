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
#include <Common/platform/assert.h>
#include <Common/StringLib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

namespace tas
{

static FILE* files = 0;
static char dump_file[256] = "dump.log";

static const char* file_r = 0;
static uint line_r = 0;
static uint mode = 2; /// bit flags, 1 file, 2 console

void assert_mode(uint _mode)
{
	mode = _mode;
}

void assert_definition(const char* expression, const char* file, uint line)
{
	stdprintf("\n======================================================\n");
	stdprintf("Program stopped\nAssertion: %s\nFile: %s\nLine: %u\n", expression, _file_name(file), line);
	stdprintf("======================================================\n\n");

	if(mode & 1)
	{
		if(!files)
			files = fopen(dump_file, "w");
		fprintf(files, "\n======================================================\n");
		fprintf(files, "Program stopped\nAssertion: %s\nFile: %s\nLine: %u\n", expression, _file_name(file), line);
		fprintf(files, "======================================================\n\n");
	}

	/* EXIT */
	exit(1);
}

void assert_line_reg(const char* file, uint line)
{
	file_r = file;
	line_r = line;
}

void assert_line(const char* file, uint line)
{
	if(mode & 1)
	{
		if(!files)
			files = fopen(dump_file, "w");
		fprintf(files, "%5u | %s\n", line, _file_name(file));
	}
	if(mode & 2)
		printf("%5u | %s\n", line, _file_name(file));
}

void assert_print(char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
	char buffer[256];
	vsprintf(buffer, fmt, args);
	va_end(args);

	if(mode & 1)
	{
		if(!files)
			files = fopen(dump_file, "w");

		fprintf(files, "%5u | ", line_r);
		fprintf(files, buffer);

		fclose(files);
		files = fopen(dump_file, "a");
	}
	if(mode & 2)
	{
		printf("%5u | ", line_r);
		printf(buffer);
	}
}

void assert_dump_file(char* file)
{
	stdcpy(dump_file, file);
}

void* assert_file()
{
	return files;
}

}