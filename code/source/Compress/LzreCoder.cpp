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
/**
 --------------------------------------------------------------------
 |               § LZRE compression algorithm §                     |
 |                                                                  |
 | - Prefix 4 types for coding token <distance, length>             |
 | - Range coder to reduce redundancy of different data types       |
 | - Minimum match length [4, 8]                                    |
 | - Multithreading       [1, 2]                                    |
 | - Price table          level 5                                   |
 | - Repeat distances     level 6                                   |
 | - Reduce distances     level 7                                   |
 | - Non greedy parsing   level 8                                   |
 | - Context mixing                                                 |
 --------------------------------------------------------------------
*/

#include <Compress/LzreCoder.h>
#include <Compress/BinaryCoder.h>
#include <Common/Bitwise.h>
#include <Common/Hash.h>
#include <Common/Math.h>
#include <Common/container/Array.h>
#include <Common/container/HashTable.h>
#include <Common/container/RingBuffer.h>

#ifdef TAA_ARCHIVARIUS_THREAD
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/Atomic.h>
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#include <Common/Atomix.h>
#endif
#endif

namespace tas
{

// LZRE_LOG bit flags, not nil enable, 2 matches, 4 literals
// REPEATN repeat distances count
// DISTANCES main distances type
#define LZRE_LOG 0
#define REPEATN 8
#define DISTANCES RingBuffer<uint>
#define DISTANCEW Array<uint>
#define HASHR(s) (s[0] << 4 ^ s[1])
#define HASHP(s) HashLy(s, prefixMain)
#define HASHL(h, b) ((h * 1664525) + b + 1013904223)
#define CHSUM(s) prefixChecksum(s)
#define REDTS 0x1000
#define REDLS 0x10000
#define REDLM 0xFFFF

enum MatchType
{
	MT_LITERA = 0,
	MT_MATCH  = 1,
	MT_REPEAT = 2,
	MT_ROLZ   = 3,
};

enum ContextType
{
	CT_PREFIX       = 0,
	CT_LITERA       = 1,
	CT_SLOT         = 2,
	CT_DISTANCE     = 3,
	CT_REPEAT       = 4,
	CT_ROLZ         = 5,
	CT_LENGTH_TYPE  = 6,
	CT_LENGTH       = 7,
	CT_LENGTH_SHORT = 8,
};

enum NonGreedy
{
	NG_ENABLE = 1,
	NG_SEARCH = 2,
	NG_OUTPUT = 3
};

enum ThreadState
{
	TS_EXIT = 0,
	TS_IDLE = 1,
	TS_RUN  = 2
};

static LzreCoder::impl* coder = 0;

#include "LzreThread.hpp"
#if LZRE_LOG
#include <stdio.h>
#endif

// Match
struct LzreMatch
{
	uint distance; // position inside sliding window
	uint length;   // full length
	byte lengthShort; // short length state
	byte distanceBit;
	byte lengthBit;
	byte type;
	LzreMatch()
	{
		reset();
	}
	void reset()
	{
		distance = 0;
		length = 0;
		lengthShort = 0;
		distanceBit = 0;
		lengthBit = 0;
		type = 0;
	}
};

// Dictionary value
struct LzreDictValue
{
	uint distance;
	uint chsum; // prefix
	uint count; // distances
	LzreDictValue()
	{
		distance = 0;
		chsum = 0;
		count = 0;
	}
	~LzreDictValue()
	{
	}
	void reset(uint checksum)
	{
		distance = 0;
		chsum = checksum;
		count = 0;
	}
	uint priority()
	{
		return count;
	}
	uint checksum()
	{
		return chsum;
	}
};

struct Counter
{
	byte n[2];
	byte ch; // upper 8 bits of hash
	Counter()
	{
		ch = 0;
		n[0] = 0;
		n[1] = 0;
	}
	~Counter()
	{
	}
	void reset(byte _ch = 0)
	{
		ch = _ch;
		n[0] = 0;
		n[1] = 0;
	}
	// update y bit
	void update(byte y)
	{
		if(n[y] < 250)
			n[y]++;
		byte r = 1 - y;
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

struct LzreCoder::impl: public IStream
{
	LzreMatch match; // founded match

	HashTable<LzreDictValue> searchDict; // search dictionary, keep recent distance
	uint* searchLink;  // dictionary link table, keep previous distances
	uint* reduceTable; // hash table, size 4 kb, keep context recent distance
	uint* reduceLink;  // link table, size 64 kb, keep previous distances
	LzreThread lzreThread;

	BinaryCoder rangeCoder;
	half* context;      // contexts bit coding
	byte* contextBit;   // contexts bits
	uint* contextShift; // contexts distances
	uint  contextState; // contexts state
	uint  rangeMain;    // main range
	half  probMid;      // probability 0.5
	half* priceTable;   // price table

	RingBuffer<byte> searchBuffer; // search buffer
	RingBuffer<byte> lookBuffer;   // look ahead buffer
	DISTANCEW* distances;    // founded match distances
	uint searchBoundLow; // window lower position in data stream
	uint searchBoundUp;  // window upper position in data stream
	uint searchSize;     // search buffer size top
	half lookSize;       // current lab size
	half labSize;        // maximum lab size
	byte minMatchLen;    // minimum, maximum match lengths
	half maxMatchLen;
	byte maxShortLen;    // maximum value of short length
	byte lengthBit;
	half matchCycles; // count distances per prefix in hash table
	byte prefixMain;  // prefix main length
	byte prefixFound; // founded prefixes
	byte* prefixLab;  // prefix from lab
	byte mode;  // process mode
	byte level; // compression level
	byte* src;
	byte* dst;
	uint srcRead;
	uint srcAvail;
	wint srcTotal;
	uint dstWrite;
	uint dstAvail;
	wint dstTotal;
	byte* restDec;
	byte* streamDec; // decoded stream
	uint* mask;  // full mask
	uint* maskf; // flag mask
	half* lengths;
	byte eof;    // end of file
	byte flush;  // flush data to output stream
	byte repeats; // repeat matches state, level 6
	byte repIndex; // repeat match index
	byte repIndexBit; // repeat index bit
	DISTANCES repDist;  // repeat distances
	LzreDictValue** dn; // dictionary nodes pointers
	byte threadCount; // current threads count
	byte threadMax;   // maximum threads count
	byte threadRun;   // 0 main thread, 1 all threads
	byte prices;      // level 5
	byte rolz;        // reduce distances, level 7
	byte ngreedy;     // non greedy parsing level 8
	uint pmLen;       // context length type
	byte rolzBit;     // reduce context bits
	half rolzCount;   // reduce context count
	byte* prefixCode;   // prefix codes
	byte* prefixBit;    // prefix bits
	byte  prefixBitMax; // prefix bits max
	byte  prefixCount;  // prefix count matches
	uint distance;    // inside window
	uint distancem;   // inside stream
	byte findOpt;     // result optimal match search
	uint* distanceOpts;
	uint* lengthOpts;
	half rolzIndex;
	half* pricem;
	uint* stats;
	uint* statsd;
	uint* red;

	/* context mixing */
	HashTable<Counter> contextTable;
	byte contextLen;
	uint* hashy; // context [0, 3]
	uint* hashes;
	uint* weight;
	byte cmix; // model state

#if LZRE_LOG
	FILE* log;
	FILE* logc;
#endif

	int compress   (CoderStream* sm, byte flush);
	int uncompress (CoderStream* sm, byte flush);

	int initialise(LzreParameters* params);
	int initialiseContext();
	int initialiseContextMix();
	int initialisePrefix();

	int updateLab();
	int findMatch();
	int findMatchThread(byte threadn);
	int findMatchOptimal();
	int findMatchOptimalPrice(byte state);
	int writeMatch();
	int updateContextState(byte mode);
	int updateDictionary();
	int normaliseDictionary();
	int putEof();
	int decodePrefix(byte prefix);
	int logMessage();
	int updateContext(half* cont, byte bit);
	uint getPrice(uint code, byte bitn, byte type, uint model);
	uint getPriceDistance(uint distance);
	uint prefixChecksum(byte* p);

	byte inputByte();
	void outputByte(byte b);

	int writeCode(uint code, byte bitn, byte type, uint model = 1);
	int writeCodeDistance(uint distance);
	uint readCode(byte bitn, byte type, uint model = 1);
	uint readCodeBit(half* probs, uint model);
	uint readCodeDistance();

	void predict(uint& n0, uint& n1, byte c0);
	int writeCodeLit();
	byte readCodeLit();
};

LzreCoder::LzreCoder()
{
	_m = new impl;
	coder = _m;
	_m->searchLink = 0;
	_m->lookSize = 0;
	_m->labSize = 0;
	_m->minMatchLen = 0;
	_m->lengthBit = 0;
	_m->searchBoundLow = 1;
	_m->searchBoundUp = 0;
	_m->searchSize = 0;
	_m->maxMatchLen = 0;
	_m->maxShortLen = 0;
	_m->matchCycles = 0;
	_m->prefixMain = 4;
	_m->prefixFound = 0;
	_m->level = 0;
	_m->streamDec = 0;
	_m->srcTotal = 0;
	_m->dstTotal = 0;
	_m->dst = 0;
	_m->dstWrite = 0;
	_m->srcAvail = 0;
	_m->srcRead = 0;
	_m->dstAvail = 0;
	_m->src = 0;
	_m->repIndex = 0;
	_m->repIndexBit = 3;
	_m->repeats = 0;
	_m->eof = 0;
	_m->flush = 0;
	_m->threadRun = 0;
	_m->threadCount = 1;
	_m->threadMax = 0;
	_m->restDec = 0;
	_m->context = 0;
	_m->contextBit = 0;
	_m->contextShift = 0;
	_m->contextState = 0;
	_m->rangeMain = 0;
	_m->probMid = 0;
	_m->priceTable = 0;
	_m->rolz = 0;
	_m->ngreedy = 0;
	_m->prices = 0;
	_m->mode = 0;
	_m->reduceTable = 0;
	_m->reduceLink = 0;
	_m->lengths = 0;
	_m->distances = 0;
	_m->rolzBit = 0;
	_m->rolzCount = 0;
	_m->hashy = 0;
	_m->hashes = 0;
	_m->weight = 0;
	_m->contextLen = 0;
	_m->cmix = 0;
	_m->dn = new LzreDictValue*[2];
	_m->stats = new uint[10];
	_m->statsd = new uint[30];
	_m->mask = new uint[33];
	_m->maskf = new uint[33];
	_m->prefixLab = new byte[20];
	_m->prefixCode = new byte[4];
	_m->prefixBit = new byte[4];
	_m->pmLen = 1;
	_m->prefixBitMax = 0;
	_m->prefixCount = 0;
	_m->findOpt = 0;
	_m->distance = 0;
	_m->distancem = 0;
	_m->rolzIndex = 0;
	_m->red = 0;
	_m->pricem = 0;
	_m->distanceOpts = 0;
	_m->lengthOpts = 0;

	forn(33)
	{
		if(i < 2)
		{
			_m->dn[i] = 0;
			_m->mask[i] = i;
			_m->maskf[i] = i;
		}
		else
		{
			_m->mask[i] = ((u64) 1 << i) - 1;
			_m->maskf[i] = 1 << i - 1;
		}
		if(i < 30)
			_m->statsd[i] = 0;
		if(i < 10)
		{
			_m->stats[i] = 0;
			_m->prefixLab[i] = 0;
		}
	}
#if LZRE_LOG
	_m->log = 0;
	_m->logc = 0;
#endif
}

LzreCoder::~LzreCoder()
{
#if LZRE_LOG
	uint total = 0;
	forn(3) total += _m->stats[i+1];

	fprintf(_m->log, "-- data types --\n\n");

	fprintf(_m->log, "literal %u\n", _m->stats[0]);
	fprintf(_m->log, "match   %u\n", _m->stats[1]);
	fprintf(_m->log, "repeat  %u\n", _m->stats[2]);
	fprintf(_m->log, "rolz    %u\n", _m->stats[3]);

	fprintf(_m->log, "\n-- matches --\n\n");

	fprintf(_m->log, "summary %u\n", total);
	fprintf(_m->log, "ngreedy %u\n", _m->stats[4]);
	fprintf(_m->log, "optimal %u\n", _m->stats[8]);
	fprintf(_m->log, "short   %u\n", _m->stats[7]);

	fprintf(_m->log, "\n-- lengths --\n\n");

	fprintf(_m->log, "encoded %u\n", _m->stats[5]);
	fprintf(_m->log, "default %u\n", total - _m->stats[5]);
	fprintf(_m->log, "minimum %u", _m->stats[6]);

	fprintf(_m->log, "\n\n-- distances --\n\n");
	forn(30) fprintf(_m->log, "[%02u] %u\n", i, _m->statsd[i]);
#endif

	safe_delete_array(_m->searchLink);
	safe_delete_array(_m->streamDec);
	safe_delete_array(_m->context);
	safe_delete_array(_m->contextBit);
	safe_delete_array(_m->contextShift);
	safe_delete_array(_m->priceTable);
	safe_delete_array(_m->mask);
	safe_delete_array(_m->maskf);
	safe_delete_array(_m->distances);
	safe_delete_array(_m->lengths);
	safe_delete_array(_m->stats);
	safe_delete_array(_m->statsd);
	safe_delete_array(_m->prefixLab);
	safe_delete_array(_m->restDec);
	safe_delete_array(_m->dn);
	safe_delete_array(_m->prefixCode);
	safe_delete_array(_m->prefixBit);
	safe_delete_array(_m->hashy);
	safe_delete_array(_m->hashes);
	safe_delete_array(_m->weight);
	safe_delete_array(_m->pricem);
	safe_delete_array(_m->distanceOpts);
	safe_delete_array(_m->lengthOpts);
	safe_delete_array(_m->reduceTable);
	safe_delete_array(_m->reduceLink);

	_m->lzreThread.uninitialise();

#if LZRE_LOG
	if(_m->log)
		fclose(_m->log);
	if(_m->logc)
		fclose(_m->logc);
#endif
	safe_delete(_m);
}

int LzreCoder::impl::initialise(LzreParameters* params)
{
	uint dictSize = 0;
	uint minMatch = 0;
	mode = params->mode;
	dictSize = params->dictionary;
	minMatch = params->minMatch;
	maxMatchLen = params->maxMatch;
	level = params->level;
	matchCycles = params->cycles;
	rolzCount = params->rolz;
	threadMax = params->threads;
	cmix = params->cmix;
	level = clamp(level, 9, 1);

	if(!dictSize)
		dictSize = 1 << 16 << level - 1;
	else
		dictSize = clamp(dictSize, 1 << 28, 1 << 16);

	if(!maxMatchLen)
	{
		if(level < 5)
			maxMatchLen = 128;
		else
			maxMatchLen = 256;
	}
	else
		maxMatchLen = clamp(maxMatchLen, 1024, 32);

	if(mode == 1)
	{
		if(level >= 5)
			prices = 1;
		if(level >= 8)
			ngreedy = 1;
		params->maxMatch = maxMatchLen;
	}
	if(level >= 6)
		repeats = 1;
	if(level >= 7)
		rolz = 1;

	if(rolz)
	{
		reduceTable = new uint[REDTS];
		reduceLink = new uint[REDLS];
		forn(REDTS)
		reduceTable[i] = 0;
		forn(REDLS)
		reduceLink[i] = 0;
		if(!rolzCount)
		{
			rolzCount = 16;
			rolzBit = 4;
		}
		else
		{
			rolzCount = clamp(rolzCount, 256, 16);
			rolzBit = bitGreat(rolzCount - 1);
		}
		params->rolz = rolzCount;
	}
	else
		params->rolz = 0;

	if(repeats)
		repDist.initialise(REPEATN);

	if(mode == 1)
	{
		if(minMatch == 0)
			minMatch = 4;
		prefixMain = clamp(minMatch, 4, 2);
		minMatchLen = clamp(MAX(minMatch, prefixMain), 8, 4);

		if(!matchCycles)
		{
			int levels[] = {1, 4, 8, 16, 32, 48, 64, 96, 128};
			matchCycles = levels[level - 1];
		}
		else
			matchCycles = clamp(matchCycles, 1 << 12, 1);
		params->cycles = matchCycles;
	}
	else
		minMatchLen = minMatch;

	lengthBit = bitGreat(maxMatchLen - 1);
	maxShortLen = mask[lengthBit >> 1] + minMatchLen - 1;
	maxMatchLen = mask[lengthBit] + maxShortLen;
	labSize = maxMatchLen + 10;

	searchBuffer.initialise(dictSize);
	forn(4) searchBuffer.push(0);
	searchBoundUp = searchBuffer.cnt + 1;
	searchSize = dictSize + 1;

	if(mode == 1)
	{
#if LZRE_LOG
		log = fopen("Notification\\lzre_e.log", "wb");
#if LZRE_LOG > 1
		logc = fopen("Notification\\lzre_ec.log", "wb");
#endif
#endif
		mode = 0;
		lookBuffer.initialise(labSize);
		if(prefixMain == 2)
			searchDict.initialise(1 << 16);
		else
			searchDict.initialise(1 << prefixMain + 15);
		searchLink = new uint[dictSize];
		forn(dictSize)
		searchLink[i] = 0;
#if defined(TAA_ARCHIVARIUS_THREAD) and TAA_PLATFORM != TAA_PLATFORM_UNKNOWN
		if(ngreedy)
			threadMax = clamp(threadMax, 2, 1);
		else
			threadMax = 1;
#else
		threadMax = 1;
#endif
		pricem = new half[3];
		lengths = new half[2];
		distances = new DISTANCEW[2];
		distanceOpts = new uint[3];
		lengthOpts = new uint[3];
		forn(2) distances[i].reserve(matchCycles * 2);
		if(threadMax > 1)
			lzreThread.initialise(threadMax - 1);
		params->dictionary = dictSize;
		params->minMatch = minMatchLen;
		params->suffix = 1;
		params->threads = threadMax;
	}
	else // uncompress
	{
#if LZRE_LOG
		log = fopen("Notification\\lzre_d.log", "wb");
#if LZRE_LOG > 1
		logc = fopen("Notification\\lzre_dc.log", "wb");
#endif
#endif
		mode = 1;
		streamDec = new byte[maxMatchLen + 20];
		streamDec[0] = 0;
	}

	initialisePrefix();
	initialiseContext();
	initialiseContextMix();
	return 1;
}

int LzreCoder::impl::initialiseContext()
{
	// Context coding bits.
	// Best predictions given
	// most significant bits.
	// ----------------------------------
	// | i | description    | min | max |
	// |--------------------------------|
	// | 0 | prefix         |  3  |     |
	// | 1 | litera         | 16  |     |
	// | 2 | distance slot  |  6  |     |
	// | 3 | distance       |  8  | 12  |
	// | 4 | repeat index   |  3  |     |
	// | 5 | rolz index     |  4  |     |
	// | 6 | length type    |  4  |     |
	// | 7 | length         |  8  | 10  |
	// | 8 | length short   |  5  |     |
	// ----------------------------------

	uint contextTypes = 9;
	uint contextTotal = 0;
	uint contextSpace[9] = { 3, 16, 6, 8, 3, 4, 4, 8, 5 };

	if(level > 5)
	{
		byte extra = level - 5;
		contextSpace[CT_DISTANCE] += extra;
		contextSpace[CT_LENGTH] += extra / 2;
		if(rolz)
			contextSpace[CT_ROLZ] = rolzBit;
		if(cmix)
			contextSpace[CT_LITERA] = 0;
	}

	uint contextCount[9] = {0};
	forn(contextTypes) contextCount[i] = maskf[contextSpace[i] + 1];

	// prefixes contexts for 8 states
	contextCount[CT_PREFIX] += maskf[contextSpace[CT_PREFIX] + 1] * 7;

	// lengths contexts for all match types
	if(prefixCount > 1)
	{
		forn(3) contextCount[CT_LENGTH_TYPE + i] += maskf[contextSpace[CT_LENGTH_TYPE + i] + 1] * 2;
	}

	contextShift = new uint[contextTypes];
	contextBit = new byte[contextTypes];

	forn(contextTypes)
	{
		contextBit[i] = contextSpace[i];
		contextShift[i] = contextTotal;
		contextTotal += contextCount[i];
	}

	assert(contextTotal < 1 << 20);
	context = new half[contextTotal];

	byte rangeBits = 12;
	rangeMain = 1 << rangeBits;
	probMid = 1 << rangeBits - 1;

	forn(contextTotal)
	context[i] = probMid;

	if(prices)
	{
		priceTable = new half[rangeMain];
		menset(priceTable, 0, rangeMain * 2);
		forn(rangeMain - 4)
		priceTable[i + 2] = -tas::log2(float(i + 2) / rangeMain) * 100;
	}

	rangeCoder.initialise(rangeBits);
	rangeCoder.stream = this;
	return 1;
}

int LzreCoder::impl::initialiseContextMix()
{
	if(!cmix) return 0;
	contextLen = 4;
	contextTable.initialise(1 << 21);
	hashy = new uint[contextLen];
	hashes = new uint[contextLen];
	weight = new uint[contextLen];
	forn(contextLen)
	{
		hashy[i] = 0;
		hashes[i] = 0;
		weight[i] = (i+1)*(i+1);
	}
	return 1;
}

int LzreCoder::impl::initialisePrefix()
{
	forn(4)
	{
		prefixCode[i] = 0;
		prefixBit[i] = 0;
	}

	prefixCount = 1;
	prefixBitMax = 1;
	prefixCode[MT_LITERA] = 0; // 0
	prefixCode[MT_MATCH]  = 1; // 1
	prefixBit [MT_LITERA] = 1;
	prefixBit [MT_MATCH]  = 1;

	if(repeats)
	{
		prefixCount = 2;
		prefixBitMax = 2;
		prefixCode[MT_MATCH] = 2; // 10
		prefixBit [MT_MATCH] = 2;
		if(rolz)
		{
			prefixCount = 3;
			prefixBitMax = 3;
			prefixCode[MT_REPEAT] = 6; // 110
			prefixCode[MT_ROLZ]   = 7; // 111
			prefixBit [MT_ROLZ]   = 3;
			prefixBit [MT_REPEAT] = 3;
		}
		else
		{
			prefixCode[MT_REPEAT] = 3; // 11
			prefixBit [MT_REPEAT] = 2;
		}
	}
	return 1;
}

uint LzreCoder::impl::getPrice(uint code, byte bitn, byte type, uint model)
{
	byte bit = 0;
	uint msb = maskf[bitn];
	uint price = 0;
	half* probs = context + contextShift[type];
	forn(bitn)
	{
		model &= mask[contextBit[type]];
		bit = (code & msb) != 0;
		if(!bit)
			price += priceTable[probs[model]];
		else
			price += priceTable[rangeMain - probs[model]];
		model += model + bit;
		msb >>= 1;
	}
	return price;
}

uint LzreCoder::impl::getPriceDistance(uint distance)
{
	uint price = 0;
	uint slot = 0;
	if(distance < 4)
	{
		slot = distance;
		price = getPrice(slot, 6, CT_SLOT, 1);
	}
	else
	{
		slot = bitGreat(distance);
		uint extra = slot - 2;
		slot = extra + 1 << 1 | distance >> extra & 1;
		price = getPrice(slot, 6, CT_SLOT, 1);
		price += getPrice(distance, extra, CT_DISTANCE, 1);
	}
	return price;
}

int LzreCoder::impl::updateContext(half* proby, byte bit)
{
	uint prob = *proby;
	if(!bit)
		prob += (rangeMain - prob) >> 5;
	else
		prob -= prob >> 5;
	*proby = prob;
	return 1;
}

byte LzreCoder::impl::inputByte()
{
	byte c = 0;
	if(restDec and restDec[0])
		c = restDec[restDec[0]--];
	else if(srcRead < srcAvail)
		c = src[srcRead++];
	return c;
}

void LzreCoder::impl::outputByte(byte b)
{
	assert(dstWrite < dstAvail);
	dst[dstWrite++] = b;
}

int LzreCoder::impl::writeCode(uint code, byte bitn, byte type, uint model)
{
	byte bit = 0;
	uint msb = maskf[bitn];
	half* probs = context + contextShift[type];
	forn(bitn)
	{
		model &= mask[contextBit[type]];
		bit = (code & msb) != 0;
		rangeCoder.encodeBit(probs[model], bit);
		updateContext(probs + model, bit);
		model += model + bit;
		msb >>= 1;
	}
	return 1;
}

int LzreCoder::impl::writeCodeDistance(uint distance)
{
	uint slot = 0;
	if(distance < 4)
	{
		slot = distance;
		writeCode(slot, 6, CT_SLOT, 1);
		match.distanceBit = 6;
	}
	else
	{
		slot = bitGreat(distance);
		uint extra = slot - 2;
		slot = extra + 1 << 1 | distance >> extra & 1;
		writeCode(slot, 6, CT_SLOT, 1);
		writeCode(distance, extra, CT_DISTANCE, 1);
		match.distanceBit = extra + 6;
#if LZRE_LOG
		if(extra + 6 < bitGreat(searchBuffer.cnt - 1)) stats[8]++;
#endif
	}
	return 1;
}

uint LzreCoder::impl::readCode(byte bitn, byte type, uint model)
{
	byte bit = 0;
	uint code = 0;
	half* probs = context + contextShift[type];
	forn(bitn)
	{
		model &= mask[contextBit[type]];
		bit = rangeCoder.decodeBit(probs[model]);
		updateContext(probs + model, bit);
		model += model + bit;
		code += code + bit;
	}
	return code;
}

uint LzreCoder::impl::readCodeBit(half* probs, uint model)
{
	byte bit = rangeCoder.decodeBit(probs[model]);
	updateContext(probs + model, bit);
	return bit;
}

uint LzreCoder::impl::readCodeDistance()
{
	uint slot = readCode(6, CT_SLOT, 1);
	if(slot < 4)
	{
		match.distanceBit = 6;
		return slot;
	}
	uint extra = (slot >> 1) - 1;
	distance = (2 | slot & 1) << extra;
	distance += readCode(extra, CT_DISTANCE, 1);
	match.distanceBit = extra + 6;
	return distance;
}

void LzreCoder::impl::predict(uint& n0, uint& n1, byte c0)
{
	uint h0 = 0;
	uint h1 = 0;
	uint w = 0;
	Counter* counter;
	uint hashKey;
	forn(contextLen)
	{
		h0 = 0;
		h1 = 0;

		hashKey = HASHL(hashy[i], c0);
		counter = &contextTable.get(hashKey, hashKey >> 24);

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

int LzreCoder::impl::writeCodeLit()
{
	byte c = lookBuffer.get(0);
	byte y = 0;    // bit
	byte c0 = 1;   // context
	byte c1 = 128; // mask
	uint n0 = 1;   // count 0
	uint n1 = 1;   // count 1
	uint f0 = 0;

	// calculate context hash
	forn(contextLen - 1)
	hashy[i+1] = HASHL(hashy[i], searchBuffer.last(i));

	// each bit from MSB
	forn(8)
	{
		y = (c & c1) != 0;

		n0 = 1;
		n1 = 1;

		// prediction bits probability
		predict(n0, n1, c0);

		// encode bit in range coder
		f0 = rangeMain * n0 / (n0+n1);
		rangeCoder.encodeBit(f0, y);

		// update context, shift mask to low
		c0 += c0 + y;
		c1 >>= 1;

		// update counters in hash table
		form(u, contextLen)
		contextTable.get(hashes[u], hashes[u] >> 24).update(y);
	}

	return 1;
}

byte LzreCoder::impl::readCodeLit()
{
	byte c = 0;    // cur byte
	byte y = 0;    // bit
	byte c0 = 1;   // context
	byte c1 = 128; // mask
	uint n0 = 1;   // count 0
	uint n1 = 1;   // count 1
	uint f0 = 0;

	// calculate context hash
	forn(contextLen - 1)
	hashy[i+1] = HASHL(hashy[i], searchBuffer.last(i));

	// each bit from MSB
	forn(8)
	{
		y = (c & c1) != 0;

		n0 = 1;
		n1 = 1;

		// prediction bits probability
		predict(n0, n1, c0);

		// encode bit in range coder
		f0 = rangeMain * n0 / (n0+n1);
		y = rangeCoder.decodeBit(f0);

		// update context, shift mask to low
		c0 += c0 + y;
		c1 >>= 1;

		// update counters in hash table
		form(u, contextLen)
		contextTable.get(hashes[u], hashes[u] >> 24).update(y);
	}

	return c0;
}

uint LzreCoder::impl::prefixChecksum(byte* p)
{
	if(prefixMain == 4)
		return (uint) p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
	else if (prefixMain == 3)
		return (uint) p[0] | p[1] << 8 | p[2] << 16;
	else if (prefixMain == 2)
		return (uint) p[0] | p[1] << 8;
	return 0xFF;
}

int LzreCoder::impl::updateLab()
{
	// fill look ahead buffer
	half ln = labSize - lookSize;
	if(srcRead < srcAvail)
	{
		form(u, ln)
		{
			if(srcRead == srcAvail)
			{
				if(flush and lookBuffer.full())
					lookBuffer.pop(ln - u);
				break;
			}
			lookBuffer.push(src[srcRead++]);
			lookSize++;
		}
	}
	else
	{
		if(match.length)
			lookBuffer.pop(match.length);
		else
			lookBuffer.pop(1);
	}
	return 1;
}

int LzreCoder::impl::findMatch()
{
	match.reset();
	byte mainloop = ngreedy ? 2 : 1;
	byte state = 0;
	prefixFound = 1;

	// output non greedy match
	if(ngreedy == NG_OUTPUT)
	{
		if(findMatchOptimal() != 0)
			return 1;
	}

	if(searchBuffer.cnt < prefixMain + minMatchLen)
		return 0;

	if(lookSize < prefixMain + minMatchLen)
		return 0;

	if(ngreedy)
		ngreedy = NG_ENABLE;

	forn(2)
	{
		dn[i] = 0;
		lengths[i] = 0;
	}

	forn(prefixMain + 1)
	prefixLab[i] = lookBuffer.get(i);

	uint skip = 0;
	// search a few look ahead buffer prefixes in dictionary
	form(j, mainloop)
	{
		skip = 0;
		byte* prefixLabw = prefixLab + j;
		dn[j] = &searchDict.find(HASHP(prefixLabw), CHSUM(prefixLabw));
		if(searchDict.index == MAX_UINT32 or dn[j]->count == 0)
			skip = 1;
		else if(dn[j]->distance < searchBoundLow)
		{
			skip = 1;
			dn[j]->reset(0);
		}
		if(skip)
		{
			dn[j] = 0;
			if(!j)
				return 0;
			continue;
		}
		if(j and dn[j] == dn[0])
		{
			dn[j] = 0;
			continue;
		}
	}

	// enable non greedy search
	if(dn[1])
	{
		ngreedy = NG_SEARCH;
		prefixFound = 2;
	}

	threadRun = 0;
	threadCount = 1;

	if(ngreedy == NG_SEARCH)
	{
		threadCount = 2;
		if(threadMax > 1)
		{
			threadRun = 1;
			threads_state[0]++;
		}
		else
			findMatchThread(1);
	}

	// search from main thread
	findMatchThread(0);

	// wait second threads
	if(threadRun)
	{
		state = TS_RUN;
		while(state == TS_RUN)
			state = threads_state[0].get();
	}

	if(lengths[0] == 0 and lengths[1] == 0)
		return 0;

	// was non greedy search
	if(ngreedy == NG_SEARCH)
	{
		// non greedy success, write distances from second thread
		if(lengths[1] > lengths[0] + 1)
		{
			ngreedy = NG_OUTPUT;
			distances[0] = distances[1];
			return 0;
		}
		else
			match.length = lengths[0];
	}
	else
	{
		match.length = lengths[0];
		if(threadCount > 1 and lengths[1] != 0)
		{
			forn(distances[1].size())
			distances[0].push_back(distances[1][i]);
			if(lengths[1] > lengths[0])
				match.length = lengths[1];
		}
	}

	if(!match.length)
		return 0;

	return findMatchOptimal();
}

int LzreCoder::impl::findMatchThread(byte thread)
{
	half lenStart = prefixMain;
	half lenTop = minMatchLen;
	half lenActual = 0;
	half lenPeak = 0;
	uint distance = 0;
	uint distancem = 0;
	byte node = 0;
	lengths[thread] = 0;
	DISTANCEW& distancesw = distances[thread];
	distancesw.resize(0);

	if(ngreedy == NG_SEARCH)
		node = thread;

	lenPeak = MIN(maxMatchLen, lookSize - node);

	distancem = dn[node]->distance;
	uint count = dn[node]->count;
	uint* distancep = 0;

	// compare each byte after prefix
	for(int t = 0; t < count; t++)
	{
		if(distancem < searchBoundLow)
		{
			dn[node]->count = t;
			*distancep = 0;
			break;
		}
		distance = distancem - searchBoundLow;
		assert(distancem >= searchBoundLow);
		lenActual = lenStart;

		// search maximum length
		for(uint i = lookBuffer.beg + node + lenActual,
		        j = searchBuffer.beg + distance + lenActual;
		        lenActual < lenPeak and
		        lookBuffer.getp(i) == searchBuffer.getp(j);
		        lenActual++);

		// save distance, length
		if(lenActual >= lenTop)
		{
			distancesw.push_back(distancem);
			distancesw.push_back(lenActual);
			lenTop = lenActual;
		}

		// next
		distancem = distancem - searchBoundLow + searchBuffer.beg;
		if(distancem >= searchBuffer.bsz)
			distancem -= searchBuffer.bsz;
		distancep = searchLink + distancem;
		distancem = *distancep;
	}

	if(distancesw.size())
		lengths[thread] = lenTop;

	return lengths[thread] != 0;
}

int LzreCoder::impl::findMatchOptimal()
{
	if(ngreedy == NG_OUTPUT)
	{
		match.length = lengths[1];
#if LZRE_LOG
		stats[4]++;
#endif
	}

	red = 0;
	if(rolz)
	{
		forn(2)
		prefixLab[i] = searchBuffer.last(1 - i);
		red = &reduceTable[HASHR(prefixLab)];
		if(*red < searchBoundLow)
		{
			*red = 0;
			red = 0;
		}
	}

	// reset values
	findMatchOptimalPrice(0);

	forn(distances[0].size())
	{
		distancem = distances[0][i];
		lengths[0] = distances[0][++i];
		if(match.length > lengths[0] + 2) continue;
		distance = searchBoundUp - distancem - 1;
		if(match.length < lengths[0] + 2)
			findMatchOptimalPrice(7);
		else
			findMatchOptimalPrice(3);
	}

	if((findOpt & 8) == 0)
	{
		if(ngreedy)
			ngreedy = NG_ENABLE;
		match.length = 0;
		return 0;
	}

	// determine optimal match type
	findMatchOptimalPrice(8);

	if(findOpt & 1)
	{
		match.distance = distanceOpts[0];
		match.distanceBit = rolzBit;
		match.type = MT_ROLZ;
		match.length = lengthOpts[0];
	}
	else if(findOpt & 2)
	{
		match.distance = distanceOpts[1];
		match.distanceBit = repIndexBit;
		match.type = MT_REPEAT;
		match.length = lengthOpts[1];
	}
	else
	{
		match.distance = distanceOpts[2];
		match.type = MT_MATCH;
		match.length = lengthOpts[2];
	}

	if(repeats)
		repDist.push(match.distance);

	if(ngreedy)
		ngreedy = NG_ENABLE;

	return match.length != 0;
}

int LzreCoder::impl::findMatchOptimalPrice(byte statem)
{
	// state bit flags: 0 reset values; 1 reduced price; 2 repeated price;
	// 4 default price; 8 determine optimal match type
	if(statem == 0)
	{
		repIndex = -1;
		rolzIndex = -1;
		distance = 0;
		distancem = 0;
		findOpt = 0;
		forn(3)
		{
			pricem[i] = -1;
			distanceOpts[i] = -1;
			lengthOpts[i] = 0;
		}
		return 1;
	}

	if(statem & 8)
	{
		if((findOpt & 8) == 0)
		{
			findOpt = 0;
			return 0;
		}
		findOpt &= 7;
		// match type with minimum price
		if(prices)
		{
			byte findn = 0;
			half pricen = -1;
			forn(3)
			{
				if((findOpt & maskf[i+1]) == 0)
					continue;
				if(pricem[i] < pricen)
				{
					pricen = pricem[i];
					findn = maskf[i+1];
				}
			}
			findOpt = findn;
			assert(findn);
		}
		return 1;
	}

	if(ngreedy == NG_OUTPUT)
	{
		uint litPos = searchBoundUp - 1;
		if(distancem < searchBoundLow or (distancem <= litPos and litPos < distancem + lengths[0]))
			return 1;
	}

	half pricec = -1;
	byte state = 0;

	// reduced distance
	if(statem & 1 and red)
	{
		int rolzIndexLocal = -1;
		uint distancew = distancem - 2;
		uint* redp = red;
		forn(rolzCount)
		{
			if(*redp < searchBoundLow)
			{
				*redp = 0;
				break;
			}
			if(*redp == distancew)
			{
				rolzIndexLocal = i;
				break;
			}
			redp = reduceLink + (*redp & REDLM);
		}
		if(rolzIndexLocal != -1)
		{
			if(prices)
			{
				pricec = getPrice(rolzIndexLocal, rolzBit, CT_ROLZ, 1);
				state = pricec < pricem[0];
			}
			else
				state = rolzIndexLocal < rolzIndex;

			if(state)
			{
				findOpt |= 1;
				rolzIndex = rolzIndexLocal;
				distanceOpts[0] = distance;
				lengthOpts[0] = lengths[0];
				pricem[0] = pricec;
			}
		}
	}

	// repeated distance
	if(statem & 2 and repeats)
	{
		forn(REPEATN)
		{
			if(distance == repDist.get(i))
			{
				if(prices)
				{
					pricec = getPrice(i, repIndexBit, CT_REPEAT, 1);
					state = pricec < pricem[1];
				}
				else
					state = i < repIndex;

				if(state)
				{
					findOpt |= 2;
					repIndex = i;
					distanceOpts[1] = distance;
					lengthOpts[1] = lengths[0];
					pricem[1] = pricec;
				}
			}
		}
	}

	if(findOpt & 7)
		findOpt |= 8;

	if(!prices and findOpt & 3)
		return 1;

	if((statem & 4) == 0)
		return 1;

	// default distance
	if(prices)
	{
		pricec = getPriceDistance(distance);
		state = pricec < pricem[2];
	}
	else
		state = distance < distanceOpts[2];

	if(state)
	{
		findOpt |= 4;
		distanceOpts[2] = distance;
		lengthOpts[2] = lengths[0];
		pricem[2] = pricec;
	}

	if(findOpt & 7)
		findOpt |= 8;

	return 1;
}

int LzreCoder::impl::writeMatch()
{
	// prepare to read prefix
	updateContextState(0);

	// write prefix
	writeCode(prefixCode[match.type], prefixBit[match.type], CT_PREFIX, 1);

	if(match.type)
	{
		assert(lookSize >= match.length);
		lookSize -= match.length;

		byte type = CT_DISTANCE;  // context type
		uint lengthActual = 0;    // current
		uint lengthShift = 0;     // shift context
		uint lengthShiftType = 0; // shift context type
		match.lengthBit = lengthBit;
		match.lengthShort = 0;

		if(!eof)
		{
			// short length
			if(match.length <= maxShortLen)
			{
				lengthActual = match.length - minMatchLen + 1;
				match.lengthBit >>= 1;
				match.lengthShort = 1;
			}
			else
				lengthActual = match.length - maxShortLen;
		}

		if(match.type == MT_ROLZ)
		{
			writeCode(rolzIndex, rolzBit, CT_ROLZ, 1);
		}
		else if(match.type == MT_REPEAT)
		{
			writeCode(repIndex, repIndexBit, CT_REPEAT, 1);
		}
		else if(match.type == MT_MATCH)
		{
			writeCodeDistance(match.distance);
		}

		// length
		type = match.lengthShort ? CT_LENGTH_SHORT : CT_LENGTH;

		// choose length context by match type
		LENGTH_CONTEXT_PREPARE;

		// length type
		writeCode(match.lengthShort, 1, CT_LENGTH_TYPE, pmLen);

		// length
		writeCode(lengthActual, match.lengthBit, type, 1);

		// update length prefix context
		LENGTH_CONTEXT_UPDATE;
	}
	else // litera
	{
		if(cmix)
			writeCodeLit();
		else
		{
			half model = 256 + searchBuffer.last(0);
			writeCode(lookBuffer.get(0), 8, CT_LITERA, model);
		}
		lookSize--;
	}

	logMessage();

	// change context state
	updateContextState(1);

	return 1;
}

int LzreCoder::impl::updateContextState(byte mode)
{
	if(!mode)
	{
		contextShift[0] = maskf[contextBit[0] + 1] * contextState;
		return 1;
	}

	// change context state
	if(match.type)
	{
		if(match.lengthShort)
			contextState = match.type + 1; // first
		else
			contextState = match.type + 4; // last
	}
	else
	{
		if(contextState < 2)
			contextState = 0;
		else
			contextState = 1;
	}
	return 1;
}

int LzreCoder::impl::normaliseDictionary()
{
	if(searchBoundUp % REDLS == 0)
		return 0;
	uint subtraction = searchBoundLow - (searchBoundUp % REDLS);
	if(!mode)
	{
		forn(searchDict.size())
		{
			if(searchDict[i].distance < searchBoundLow)
				searchDict[i].reset(0);
			else
			{
				assert(searchDict[i].count != 0);
				searchDict[i].distance -= subtraction;
			}
		}
		forn(searchBuffer.bsz)
		{
			if(searchLink[i] < searchBoundLow)
				searchLink[i] = 0;
			else
				searchLink[i] -= subtraction;
		}
	}
	if(rolz)
	{
		forn(REDTS)
		{
			if(reduceTable[i] < searchBoundLow)
				reduceTable[i] = 0;
			else
				reduceTable[i] -= subtraction;
		}
		forn(REDLS)
		{
			if(reduceLink[i] < searchBoundLow)
				reduceLink[i] = 0;
			else
				reduceLink[i] -= subtraction;
		}
	}
	searchBoundLow -= subtraction;
	searchBoundUp -= subtraction;
	return 1;
}

int LzreCoder::impl::updateDictionary()
{
	uint index;
	uint count = match.length ? match.length : 1;

	form(u, count)
	{
		if(!mode)
			searchBuffer.push(lookBuffer.get(u));
		else
			searchBuffer.push(streamDec[u]);

		if(++searchBoundUp > searchSize)
			searchBoundLow++;

		if(rolz and searchBuffer.cnt > 2)
		{
			forn(2)
			prefixLab[i] = searchBuffer.last(1 - i);
			red = &reduceTable[HASHR(prefixLab)];
			distancem = searchBoundUp - 2;
			index = distancem & REDLM;
			if(*red < searchBoundLow)
				reduceLink[index] = 0;
			else
				reduceLink[index] = *red;
			*red = distancem;
		}

		if(!mode and searchBuffer.cnt > prefixMain)
		{
			forn(prefixMain)
			prefixLab[i] = searchBuffer.last(prefixMain - i - 1);
			dn[0] = &searchDict.get(HASHP(prefixLab), CHSUM(prefixLab));
			distancem = searchBoundUp - prefixMain;
			index = distancem - searchBoundLow + searchBuffer.beg;
			if(index >= searchBuffer.bsz)
				index -= searchBuffer.bsz;
			if(dn[0]->distance < searchBoundLow)
			{
				searchLink[index] = 0;
				dn[0]->count = 0;
			}
			else
				searchLink[index] = dn[0]->distance;
			dn[0]->distance = distancem;
			if(dn[0]->count < matchCycles)
				dn[0]->count++;
		}
	}

	if(searchBoundUp > 0xFFFF0000)
		normaliseDictionary();

	return 1;
}

int LzreCoder::impl::putEof()
{
	// EOF <0, 0>
	match.reset();
	match.type = MT_MATCH;
	eof = 1;
	if(!writeMatch())
		return 0;
	rangeCoder.flush();
	return 1;
}

int LzreCoder::impl::decodePrefix(byte prefix)
{
	uint code = 0;
	if(prefix == prefixCode[MT_LITERA])
	{
		match.type = MT_LITERA;
		if(cmix)
			code = readCodeLit();
		else
		{
			uint model = 256 + searchBuffer.last(0);
			code = readCode(8, CT_LITERA, model);
		}
		streamDec[0] = code;
	}
	else if(prefix == prefixCode[MT_MATCH])
	{
		match.type = MT_MATCH;
	}
	else if(prefix == prefixCode[MT_REPEAT])
	{
		match.type = MT_REPEAT;
		code = readCode(repIndexBit, CT_REPEAT, 1);
		repIndex = code;
		match.distanceBit = repIndexBit;
	}
	else if(prefix == prefixCode[MT_ROLZ])
	{
		match.type = MT_ROLZ;
		forn(2) prefixLab[i] = searchBuffer.last(1 - i);
		red = &reduceTable[HASHR(prefixLab)];
		code = readCode(rolzBit, CT_ROLZ, 1);
		assert(code < rolzCount);
		forn(code) red = reduceLink + (*red & REDLM);
		match.distance = searchBoundUp - *red - 3;
		match.distanceBit = rolzBit;
	}
	else
		assert(0 and "wrong prefix code");
	return 1;
}

int LzreCoder::impl::logMessage()
{
#if LZRE_LOG
	if(eof) return 1;
	if(LZRE_LOG & 2 and match.type)
	{
		fprintf(logc, "<%u %u | %u %u | %u %u", match.type, match.lengthShort,
		        match.distance, match.length, match.distanceBit, match.lengthBit);
		if(match.type == MT_REPEAT)
			fprintf(logc, " | %u %u>\n", repIndex, repIndexBit);
		else
			fprintf(logc, ">\n", repIndex, repIndexBit);
	}
	if(LZRE_LOG & 4 and match.type == MT_LITERA)
		fprintf(logc, "%c", !mode ? lookBuffer.get(0) : streamDec[0]);
	stats[match.type]++;
	if(match.type and match.lengthShort)
		stats[5]++;
	if(match.type and match.length == minMatchLen)
		stats[6]++;
	if(match.type == MT_MATCH)
		statsd[bitGreat(match.distance)]++;
#endif
	return 1;
}

int LzreCoder::impl::compress(CoderStream* sm, byte flusha)
{
	flush = flusha;

	if(eof)
		return IS_EOF_ERROR;

	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 0x1000)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;
	byte run = 1;

	while(run)
	{
		updateLab();

		// not last block, lab not filled
		if(lookSize < labSize and !flush)
		{
			run = 0;
			continue;
		}

		int find = findMatch();

		// write code to destination
		if(!writeMatch())
			return IS_STREAM_ERROR;

		// end of file
		// last block, founded all matches in lab
		if(flush and srcRead == srcAvail and !lookSize)
		{
			if(!putEof())
				return IS_STREAM_ERROR;
			run = 0;
			continue;
		}

		// update dictionary
		updateDictionary();

		// dest size small
		if(dstAvail - dstWrite < 40)
		{
			ret = IS_STREAM_END;
			run = 0;
			continue;
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

int LzreCoder::impl::uncompress(CoderStream* sm, byte flusha)
{
	flush = flusha;

	if(eof)
	{
		_line;
		return IS_EOF_ERROR;
	}

	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 0x1000)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;
	byte bit = 0;
	byte prefix = 0;
	byte prefixBits = 0;
	uint prefixModel = 1;
	half* probs = 0;

	if(!dstTotal)
		rangeCoder.initialiseDecoder();

	for(; srcRead < srcAvail;)
	{
		match.reset();
		match.lengthBit = lengthBit;

		// prepare to read prefix
		updateContextState(0);

		probs = context + contextShift[CT_PREFIX];
		prefixModel = 1;
		prefix = 0;
		prefixBits = 0;

		// read prefix code
		for(;;)
		{
			bit = readCodeBit(probs, prefixModel);
			prefixModel += prefixModel + bit;
			prefixBits++;
			if(!bit or prefixBits == prefixBitMax)
				break;
		}
		prefix = prefixModel & mask[prefixBits];

		// decode prefix code
		decodePrefix(prefix);

		if(match.type)
		{
			byte type = CT_DISTANCE;
			uint lengthShift = 0;     // shift context
			uint lengthShiftType = 0; // shift context type

			if(match.type == MT_MATCH)
				match.distance = readCodeDistance();
			else if(match.type == MT_REPEAT)
				match.distance = repDist.get(repIndex);

			// length context shift by match type
			if(match.type != MT_MATCH)
			{
				lengthShiftType = maskf[contextBit[CT_LENGTH_TYPE] + 1] * (match.type - 1);
				contextShift[CT_LENGTH_TYPE] += lengthShiftType;
			}

			// length type
			probs = context + contextShift[CT_LENGTH_TYPE];
			match.lengthShort = readCodeBit(probs, pmLen);

			type = match.lengthShort ? CT_LENGTH_SHORT : CT_LENGTH;

			// length context shift by match type
			if(match.type != MT_MATCH)
			{
				lengthShift = maskf[contextBit[type] + 1] * (match.type - 1);
				contextShift[type] += lengthShift;
			}

			if(match.lengthShort)
			{
				match.lengthBit >>= 1;
				match.length = readCode(match.lengthBit, CT_LENGTH_SHORT);
			}
			else
				match.length = readCode(match.lengthBit, CT_LENGTH);

			// update length prefix context
			LENGTH_CONTEXT_UPDATE;

			if(!match.length)
			{
				eof = 1;
				break;
			}

			if(match.lengthShort)
				match.length += minMatchLen - 1;
			else
				match.length += maxShortLen;

			if(repeats)
				repDist.push(match.distance);

			distance = searchBuffer.lastp(match.distance);
			form(u, match.length)
			streamDec[u] = searchBuffer.getp(distance);

			/* len >= 8
			dist align 4 && .bsz - dist >= m.len
				get address dict
				mencpy .... */
		}

		logMessage();

		// change context state
		updateContextState(1);

		half count = match.length ? match.length : 1;
		if(dstWrite + count > dstAvail)
			return IS_STREAM_ERROR;
		form(u, count)
		dst[dstWrite++] = streamDec[u];
		// dst align 4, mencpy ...

		updateDictionary();

		// stop
		if(srcRead == srcAvail)
			break;

		// keep some bytes for continuous decoding
		if(!flush and srcAvail - srcRead < 40)
		{
			// stack
			if(restDec == 0)
				restDec = new byte[40];
			restDec[0] = srcAvail - srcRead;
			forn(restDec[0])
			restDec[i+1] = src[srcAvail-i-1];
			break;
		}

		// destination size small
		if(dstAvail - dstWrite < maxMatchLen + 40)
		{
			ret = IS_STREAM_END;
			break;
		}
	}

	dstTotal += dstWrite;

	// last block without eof or not last block with eof
	if((srcRead == srcAvail and flush and !eof) or ((srcRead != srcAvail or !flush) and eof))
	{
		_printu(eof);
		return IS_EOF_ERROR;
	}

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

int LzreCoder::initialise(LzreParameters* params)
{
	return _m->initialise(params);
}

int LzreCoder::compress(CoderStream* sm, byte flush)
{
	return _m->compress(sm, flush);
}

int LzreCoder::uncompress(CoderStream* sm, byte flush)
{
	return _m->uncompress(sm, flush);
}

LzreParameters::LzreParameters()
{
	mode = 0;
	dictionary = 0;
	minMatch = 0;
	maxMatch = 0;
	level = 0;
	cycles = 0;
	rolz = 0;
	suffix = 0;
	threads = 0;
	cmix = 0;
}

void findMatchThread(byte id)
{
	coder->findMatchThread(id);
}

}