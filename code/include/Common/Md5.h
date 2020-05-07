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
#ifndef _TAH_MD5_h_
#define _TAH_MD5_h_

#include <Common/Config.h>

namespace tas
{

/// The MD5 Message-Digest Algorithm.
class Md5
{
public:

	Md5();
	~Md5();

	/** Initialise message digest buffer. */
	int initialise();

	/** Calculate message digest.
	  * @param block Message block data.
	  * @param length Message block length in bytes.
	  * @param end Last block in message.
	  * @return Pointer to message digest array of 16 bytes.
	  * @remark For not last input block length is 64 byte.
	  */
	byte* calculate(byte* block, uint length, byte end);

	/** Retrieve last message digest. */
	byte* getDigest();

private:

	uint* state;  /// states from a to c
	uint* table;  /// sinus table
	byte* digest; /// result md5, buffer, not string
	wint total; /// total message length in bytes
};

}

#endif