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
#ifndef _TAH_TextStream_h_
#define _TAH_TextStream_h_

#include <Common/Config.h>
#include <Common/String.h>

namespace tas
{

class TAA_LIB TextStream
{
public:

	/** Constructor.
	  * @param load Open mode, one for load, else write.
	  * @param strLen String length for reading.
	  * @param apn Append new line after writeLine().
	  */
	TextStream(const String& fname, bool load, const String& mode = "", uint strLen = 512);

	/** Constructor. */
	TextStream();

	/** Destructor. */
	~TextStream();

	/** Open file for reading, writing.
	  * @param fname File name.
	  * @param load Open mode, one for load, else write.
	  * @param mode Open mode, see fopen().
	  */
	bool open(const String& fname, bool load, const String& mode = "");

	/** Reopen file for reading, writing.
	  * @param load Open mode, one for load, else write.
	  * @param mode Open mode, see fopen().
	  */
	bool reopen(bool load, const String& mode = L"");

	/** Close file. */
	void close();

	/** Retrieve file state.
	  * @return Positive value if file opened.
	  */
	bool state();

	/** Read line from file.
	  * @return Positive if line was read.
	  */
	bool readLine();

	/** Retrieve last successfully read line. */
	char* getLine();

	/** Write line to file.
	  * @param format Formatted string.
	  */
	void writeLinef(char* format, ...);

	/** Write line to file.
	  * @param str String.
	  */
	void writeLine(char* str);

	/** Save file on disk. */
	void save();

	/** Set string length for reading. */
	void setStringLength(uint length);

	/** Set append new line mode after write one line. */
	void setAppendNewLine(bool state);

	/** Set auto save mode after write one line. */
	void setAutoSave(bool state);

	/** Set console output during write one line. */
	void setConsoleOutput(bool state);

	/** Get console output during write one line. */
	bool getConsoleOutput();

	/** Retrieve file size. */
	wint getSize();

	/** Retrieve file name. */
	String getName();

	/** File seek. */
	bool seek(int64 offset, int origin);

private:
	struct impl;
	impl* m;
};

}

#endif