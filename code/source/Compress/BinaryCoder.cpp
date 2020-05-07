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
#include <Compress/BinaryCoder.h>
#include <Compress/ICoder.h>

namespace tas
{

BinaryCoder::BinaryCoder()
{
	high = 0xFFFFFFFF;
	low = 0;
	mid = 0;
	range = 0;
	stream = 0;
}

BinaryCoder::~BinaryCoder()
{
}

void BinaryCoder::initialise(byte inRange)
{
	range = inRange;
}

void BinaryCoder::encodeBit(uint freq0, byte bit)
{
	assert(high > low);
	uint midProb = low + ((high - low >> range) * freq0);
	assert(high > midProb && midProb >= low);

	if(!bit)
		high = midProb;
	else
		low = midProb + 1;

	while((high ^ low) < 0x1000000)
	{
		stream->outputByte(high >> 24);
		high = high << 8 | 0xFF;
		low = low << 8;
	}
}

byte BinaryCoder::decodeBit(uint freq0)
{
	byte bit;
	assert(high > low);
	assert(mid >= low && mid <= high);
	uint midProb = low + ((high - low >> range) * freq0);
	assert(high > midProb && midProb >= low);

	if(mid <= midProb)
	{
		bit = 0;
		high = midProb;
	}
	else
	{
		bit = 1;
		low = midProb + 1;
	}

	while((high ^ low) < 0x1000000)
	{
		high = high << 8 | 0xFF;
		low = low << 8;
		mid = mid << 8 | stream->inputByte();
	}

	return bit;
}

void BinaryCoder::flush()
{
	forn(4) stream->outputByte(low >> (8 * (3 - i)));
}

void BinaryCoder::initialiseDecoder()
{
	forn(4) mid = mid << 8 | stream->inputByte();
}

}