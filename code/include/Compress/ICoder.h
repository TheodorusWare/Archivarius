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
#ifndef _TAH_ICoder_h_
#define _TAH_ICoder_h_

#include <Common/Config.h>

namespace tas
{

/// Success codes
/// IS_STREAM_END destination stream ended, need flush
/// source stream not read completely
#define IS_OK            1
#define IS_STREAM_END    2

/// Error codes
#define IS_STREAM_ERROR  -1
#define IS_EOF_ERROR     -2

/// Input, output coder data stream.
struct CoderStream
{
	byte* src;      /// source buffer
	uint  srcAvail; /// count available bytes in source buffer
	wint  srcTotal; /// total count readed bytes for all time

	byte* dst;      /// destination buffer
	uint  dstAvail; /// count available bytes in destination buffer for write by coder
	/// during encoding, then for read by application after encoding
	wint  dstTotal; /// total count writed bytes for all time

	CoderStream()
	{
		src = 0;
		dst = 0;
		srcAvail = 0;
		srcTotal = 0;
		dstAvail = 0;
		dstTotal = 0;
	}
};

/// Base interface for coder.
class ICoder
{
public:

	/** Compression, uncompression stream.
	  * @param flush Encoded data to destination stream at positive value. Set once.
	  * @return State code. See IC_*.
	  */
	virtual int compress    (CoderStream* sm, byte flush) = 0;
	virtual int uncompress  (CoderStream* sm, byte flush) = 0;

	virtual ~ICoder() {}
};

/// Base interface for input, output bytes.
class IStream
{
public:
	virtual byte inputByte() = 0;
	virtual void outputByte(byte b) = 0;
};

}

#endif