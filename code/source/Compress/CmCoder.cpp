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
#include <Compress/CmCoder.h>
#include <Compress/BinaryCoder.h>
#include <Common/Math.h>
#include <Common/container/HashTable.h>
#include <Common/container/RingBuffer.h>

#define HASH(h, b) ((h * 1664525) + b + 1013904223)

namespace tas
{

struct CmCounter
{
	byte n[2]; // count 0, 1 bits
	byte ch;   // upper 8 bits of hash

	CmCounter()
	{
		ch = 0;
		n[0] = 0;
		n[1] = 0;
	}

	~CmCounter()
	{
	}

	void reset(byte _ch = 0)
	{
		ch = _ch;
		n[0] = 0;
		n[1] = 0;
	}

	/// update y bit
	void update(byte y)
	{
		if(n[y] < 250)
			n[y]++;
		byte r = 1-y;
		if(n[r] > 2)
			n[r] = n[r] / 2 + 1;
	}

	uint priority()
	{
		return n[0] + n[1];
	}

	uint checksum()
	{
		return ch;
	}
};

struct CmCoder::impl: public IStream
{
	HashTable<CmCounter> hashTable;

	BinaryCoder rangeCoder;
	uint range;

	RingBuffer<byte> context; // context of 7 bytes
	uint* hash; // context [0, 7]

	CmCounter* counter;
	uint* hashes;
	uint* weight;
	byte contextLen;

	byte* src;
	byte* dst;
	uint srcAvail;
	uint srcRead;
	uint dstWrite;
	uint dstAvail;
	wint srcTotal;
	wint dstTotal;
	wint encsz;
	byte* rest;

	int initialise(CmParameters* params);

	void predict(uint& n0, uint& n1, byte c0);
	void update(byte y);

	int compress(CoderStream* sm, byte flush);
	int uncompress(CoderStream* sm, byte flush);

	byte inputByte();
	void outputByte(byte b);
};

CmCoder::CmCoder()
{
	_m = new impl;
	_m->counter = 0;
	_m->srcTotal = 0;
	_m->dstTotal = 0;
	_m->dst = 0;
	_m->dstWrite = 0;
	_m->srcAvail = 0;
	_m->srcRead = 0;
	_m->dstAvail = 0;
	_m->rest = 0;
	_m->range = 0;
	_m->encsz = 0;
	_m->hash = 0;
	_m->hashes = 0;
	_m->weight = 0;
}

CmCoder::~CmCoder()
{
	safe_delete_array(_m->hash);
	safe_delete_array(_m->hashes);
	safe_delete_array(_m->weight);
	safe_delete_array(_m->rest);
	safe_delete(_m);
}

int CmCoder::initialise(CmParameters* params)
{
	return _m->initialise(params);
}

int CmCoder::compress(CoderStream* sm, byte flush)
{
	return _m->compress(sm, flush);
}

int CmCoder::uncompress(CoderStream* sm, byte flush)
{
	return _m->uncompress(sm, flush);
}

int CmCoder::impl::initialise(CmParameters* params)
{
	byte level = clamp(params->level, 9, 1);
	uint dictSize = params->dictionary;
	contextLen = params->context;
	if(!contextLen)
		contextLen = clamp(level + 3, 8, 4);
	else
		contextLen = clamp(contextLen, 8, 4);
	if(!dictSize)
		dictSize = 1 << 16 << level - 1;
	else
		dictSize = clamp(dictSize, 1 << 24, 1 << 16);
	context.initialise(contextLen - 1);
	context.cnt = contextLen - 1;
	byte rangeBit = 16;
	range = 1 << rangeBit;
	rangeCoder.initialise(rangeBit);
	rangeCoder.stream = this;
	hashTable.initialise(dictSize);
	hash = new uint[contextLen];
	hashes = new uint[contextLen];
	weight = new uint[contextLen];
	forn(contextLen)
	{
		hash[i] = 0;
		hashes[i] = 0;
		weight[i] = (i+1)*(i+1);
	}
	params->dictionary = dictSize;
	params->context = contextLen;
	return 1;
}

byte CmCoder::impl::inputByte()
{
	byte c = 0;
	if(rest and rest[0] < rest[1])
		c = rest[2 + rest[0]++];
	else if(srcRead < srcAvail)
		c = src[srcRead++];
	return c;
}

void CmCoder::impl::outputByte(byte b)
{
	assert(dstWrite < dstAvail);
	dst[dstWrite++] = b;
}

void CmCoder::impl::predict(uint& n0, uint& n1, byte c0)
{
	uint h0 = 0;
	uint h1 = 0;
	uint w = 0;
	forn(contextLen)
	{
		h0 = 0;
		h1 = 0;

		uint hashKey = HASH(hash[i], c0);
		counter = &hashTable.get(hashKey, hashKey >> 24);

		// get counters
		h0 = counter->n[0];
		h1 = counter->n[1];

		// weight
		w = weight[i];

		// weight n^2
		if(h0)
			n0 += h0 * w;
		if(h1)
			n1 += h1 * w;

		// update hash
		hashes[i] = hashKey;
	}
}

void CmCoder::impl::update(byte y)
{
	// update contexts counters in hash table
	forn(contextLen)
	hashTable.get(hashes[i], hashes[i] >> 24).update(y);
}

int CmCoder::impl::compress(CoderStream* sm, byte flush)
{
	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 4 * KB)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;

	byte c = 0;    // cur byte
	byte y = 0;    // bit
	byte c0 = 1;   // context
	byte c1 = 128; // mask
	uint n0 = 1;   // count 0
	uint n1 = 1;   // count 1
	uint f0 = 0;

	while(srcRead < srcAvail)
	{
		c = src[srcRead++];

		c0 = 1;
		c1 = 128;

		// each bit from MSB
		forn(8)
		{
			y = (c & c1) != 0;

			n0 = 1;
			n1 = 1;

			// prediction bits probability
			predict(n0, n1, c0);

			// encode bit in range coder
			f0 = range * n0 / (n0+n1);
			rangeCoder.encodeBit(f0, y);

			// update context, shift mask to low
			c0 += c0 + y;
			c1 >>= 1;

			// update counters in hash table
			update(y);
		}

		// update context
		context.push(c0);

		// calculate context hash
		forn(contextLen - 1)
		hash[i+1] = HASH(hash[i], context.last(i));

		// dest size small
		if(dstAvail - dstWrite < 40)
		{
			// _linef;
			ret = IS_STREAM_END;
			break;
		}
	}

	if(flush and ret != IS_STREAM_END)
		rangeCoder.flush();

	dstTotal += dstWrite;
	if(ret != IS_STREAM_END)
	{
		srcTotal += srcRead;
		srcRead = 0;
	}

	sm->dstAvail = dstWrite;
	sm->srcTotal = srcTotal;
	sm->dstTotal = dstTotal;

	return ret;
}

int CmCoder::impl::uncompress(CoderStream* sm, byte flush)
{
	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 4 * KB)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;

	byte y = 0;    // bit
	byte c0 = 1;   // context
	byte c1 = 128; // mask
	uint n0 = 1;   // count 0
	uint n1 = 1;   // count 1
	uint f0 = 0;

	/* cm decoder need source file size */
	if(encsz == 0)
	{
		/* little endian */
		forn(8) encsz |= (wint)inputByte() << 8 * i;
		rangeCoder.initialiseDecoder();
	}

	while(1)
	{
		c0 = 1;
		c1 = 128;

		// each bit from MSB
		forn(8)
		{
			n0 = 1;
			n1 = 1;

			// prediction bits probability
			predict(n0, n1, c0);

			// decode bit from range coder
			f0 = range * n0 / (n0+n1);
			y = rangeCoder.decodeBit(f0);

			// update context, shift mask to low
			c0 += c0 + y;
			c1 >>= 1;

			// update counters in hash table
			update(y);
		}

		// output byte
		outputByte(c0);

		// update context
		context.push(c0);

		forn(contextLen - 1)
		hash[i+1] = HASH(hash[i], context.last(i));

		if(--encsz == 0)
			break;

		if(srcAvail - srcRead < 40 && !flush)
		{
			if(rest == 0)
				rest = new byte[40];
			rest[0] = 0;
			rest[1] = srcAvail - srcRead;
			forn(rest[1])
			rest[i+2] = src[srcAvail - rest[1] + i];
			break;
		}

		// dest size small
		if(dstAvail - dstWrite < 40)
		{
			ret = IS_STREAM_END;
			break;
		}
	}

	dstTotal += dstWrite;
	if(ret != IS_STREAM_END)
	{
		srcTotal += srcRead;
		srcRead = 0;
	}

	sm->dstAvail = dstWrite;
	sm->srcTotal = srcTotal;
	sm->dstTotal = dstTotal;

	return ret;
}

CmParameters::CmParameters()
{
	mode = 0;
	dictionary = 0;
	context = 0;
	level = 0;
}

}