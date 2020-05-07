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
#ifndef _TAH_BinaryCoder_h_
#define _TAH_BinaryCoder_h_

#include <Common/Config.h>

namespace tas
{

class IStream;

/// Binary range coder
class BinaryCoder
{
public:
	BinaryCoder();
	~BinaryCoder();

	void initialise(byte range);
	void initialiseDecoder();
	void flush();

	void encodeBit(uint freq0, byte bit);
	byte decodeBit(uint freq0);

	IStream* stream;

private:
	uint low, high, mid, range;
};

}

#endif