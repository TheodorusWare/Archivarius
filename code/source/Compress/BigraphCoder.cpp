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
#include <Compress/BigraphCoder.h>
#include <Common/Hash.h>
#include <Common/Math.h>
#include <Common/container/HashTable.h>
#include <Common/container/Array.h>

#define HASH(s) (*(word*)s)

namespace tas
{

struct BigraphValue
{
	uint chsum; // checksum
	uint count; // n-graph
	half rep; // replace byte

	BigraphValue()
	{
		chsum = 0;
		count = 0;
		rep = -1;
	}

	~BigraphValue()
	{
	}

	void reset(uint checksum)
	{
		chsum = checksum;
		count = 0;
		rep = -1;
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

struct BigraphCoder::impl
{
	Array<BigraphValue> ngraphTable;
	HashTable<BigraphValue> hashTable;
	BigraphValue* ngraphTableUn; // decode
	BigraphValue* node;
	byte ngraph; // count [2, 4]
	byte* ngb; // n-graph block
	byte ngbsz; // size
	uint* frq; // frequency input symbols
	byte nil;
	byte mode;  // process mode
	uint tableSize;

	byte* src;
	byte* dst;
	uint srcAvail;
	uint srcRead;
	uint dstWrite;
	uint dstAvail;
	wint srcTotal;
	wint dstTotal;
	byte eof;

	int initialise(byte mode);

	int parsing     (CoderStream* sm, byte flush);
	int compress    (CoderStream* sm, byte flush);
	int uncompress  (CoderStream* sm, byte flush);
};

BigraphCoder::BigraphCoder()
{
	_m = new impl;
	_m->mode = 0;
	_m->nil = 0;
	_m->node = 0;
	_m->ngraph = 0;
	_m->srcTotal = 0;
	_m->dstTotal = 0;
	_m->dst = 0;
	_m->dstWrite = 0;
	_m->srcAvail = 0;
	_m->srcRead = 0;
	_m->dstAvail = 0;
	_m->src = 0;
	_m->eof = 0;
	_m->ngbsz = 0;
	_m->tableSize = 0;
	_m->ngraphTableUn = 0;
	_m->frq = 0;
	_m->ngb = new byte[4];
	forn(4) _m->ngb[i] = 0;
}

BigraphCoder::~BigraphCoder()
{
	safe_delete_array(_m->ngraphTableUn);
	safe_delete_array(_m->ngb);
	safe_delete_array(_m->frq);
	safe_delete(_m);
}

int BigraphCoder::impl::initialise(byte _mode)
{
	mode = _mode;
	ngraph = 2;
	if(mode == 1) // parsing
	{
		tableSize = 64 * KB;
		hashTable.initialise(tableSize);
		frq = new uint[256];
		forn(256)
		frq[i] = 0;
	}
	eof = 0;
	srcTotal = 0;
	srcRead = 0;
	dstWrite = 0;
	dstTotal = 0;
	ngbsz = 0;
	if(mode == 3)
		ngraphTableUn = new BigraphValue[256];
	return 1;
}

bool compare_decrease(BigraphValue o1, BigraphValue o2)
{
	return (o1.count < o2.count);
}

int BigraphCoder::impl::parsing(CoderStream* sm, byte flush)
{
	if(eof)
		return IS_EOF_ERROR;

	if(sm->src == 0 or sm->srcAvail == 0)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;

	int ret = IS_OK;
	byte run = 1;
	uint hash = 0;

	if(ngbsz == 0)
	{
		frq[src[srcRead]]++;
		ngb[ngbsz++] = ngb[srcRead++];
	}

	while(srcRead < srcAvail)
	{
		// accumulate single byte
		frq[src[srcRead]]++;
		ngb[ngbsz++] = src[srcRead++];

		// end of file
		if(flush and srcRead == srcAvail)
		{
			eof = 1;
			break;
		}

		// encoding
		hash = HASH(ngb);
		hashTable.get(hash, hash).count++;

		// shift low single byte
		ngb[0] = ngb[1];
		ngbsz--;
	}

	if(eof)
	{
		nil = 0;
		forn(256)
		{
			if(frq[i] == 0)
				nil++;
		}
		if(nil < 3)
		{
			// _printf("not having nil bytes\n");
			return IS_STREAM_ERROR;
		}

		ngraphTable.reserve(128);
		forn(tableSize)
		{
			if(hashTable[i].count > 1)
				ngraphTable.push_back(hashTable[i]);
		}
		// _printf("\n\nnil %u table %u\n\n", nil, ngraphTable.size());
		ngraphTable.sort(compare_decrease);

		// assign nil byte to ngraph
		nil = 0;
		forn(256)
		{
			if(frq[i] == 0)
			{
				hash = ngraphTable[nil++].chsum;
				hashTable.find(hash, hash).rep = i;
				if(nil == ngraphTable.size()) break;
			}
		}
	}

	srcTotal += srcRead;
	sm->srcTotal = srcTotal; // total size
	srcRead = 0;

	return ret;
}

int BigraphCoder::impl::compress(CoderStream* sm, byte flush)
{
	if(eof)
		return IS_EOF_ERROR;

	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 4 * KB)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;
	uint hash = 0;

	if(dstTotal == 0)
	{
		/*
			table to destination
			1. count, 1 byte
			2. pairs <ngraph, rep>
		*/
		dst[dstWrite++] = nil;
		forn(nil)
		{
			hash = ngraphTable[i].chsum;
			half rep = hashTable.find(hash, hash).rep;
			assert(rep != MAX_UINT16);
			byte* hp = (byte*)&hash;
			form(u, ngraph)
			dst[dstWrite++] = hp[u];
			dst[dstWrite++] = rep;
		}
	}

	while(srcRead < srcAvail)
	{
		// accumulate n bytes
		half ln = ngraph - ngbsz;
		form(u, ln)
		{
			if(srcRead == srcAvail)
				break;
			ngb[ngbsz++] = src[srcRead++];
		}

		// not get full bytes
		if(ngbsz != ngraph and !flush)
			break;

		// end of file
		if(flush and srcRead == srcAvail)
		{
			eof = 1;
			forn(ngbsz)
			dst[dstWrite++] = ngb[i];
			break;
		}

		// encoding
		hash = HASH(ngb);
		node = &hashTable.find(hash, hash);

		bool found = 1;
		if(hashTable.index == MAX_UINT32 or node->rep == MAX_UINT16)
			found = 0;

		// not found
		if(!found)
		{
			// out byte
			assert(dstWrite < dstAvail);
			dst[dstWrite++] = ngb[0];
			forn(ngraph-1)
			ngb[i] = ngb[i+1];
			ngbsz--;
		}
		else
		{
			// out single byte associated with n-graph
			assert(dstWrite < dstAvail);
			dst[dstWrite++] = node->rep;
			ngbsz = 0;
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

int BigraphCoder::impl::uncompress(CoderStream* sm, byte flush)
{
	if(eof)
		return IS_EOF_ERROR;

	if(sm->dst == 0 or sm->src == 0 or sm->srcAvail == 0 or sm->dstAvail < 4 * KB)
		return IS_STREAM_ERROR;

	src = sm->src;
	srcAvail = sm->srcAvail;
	dst = sm->dst;
	dstAvail = sm->dstAvail;
	dstWrite = 0;

	int ret = IS_OK;
	uint hash = 0;
	byte c = 0;

	if(dstTotal == 0)
	{
		/*
			read table
			1. count, 1 byte
			2. pairs <graph, rep>
		*/
		nil = src[srcRead++];
		forn(nil)
		{
			form(u, ngraph)
			ngb[u] = src[srcRead++];
			byte rep = src[srcRead++];
			hash = HASH(ngb);
			node = &ngraphTableUn[rep];
			node->rep = rep;
			node->chsum = hash;
		}
	}

	while(srcRead < srcAvail)
	{
		c = src[srcRead++];

		// c -> ngraph
		node = &ngraphTableUn[c];

		// output found ngraph
		if(node->rep == c)
		{
			byte* hp = (byte*)&node->chsum;
			forn(ngraph)
			{
				assert(dstWrite < dstAvail);
				dst[dstWrite++] = hp[i];
			}
		}
		else
		{
			assert(dstWrite < dstAvail);
			dst[dstWrite++] = c;
		}

		// end of file
		if(flush and srcRead == srcAvail)
		{
			eof = 1;
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

int BigraphCoder::initialise(byte mode)
{
	return _m->initialise(mode);
}

int BigraphCoder::parsing(CoderStream* sm, byte flush)
{
	return _m->parsing(sm, flush);
}

int BigraphCoder::compress(CoderStream* sm, byte flush)
{
	return _m->compress(sm, flush);
}

int BigraphCoder::uncompress(CoderStream* sm, byte flush)
{
	return _m->uncompress(sm, flush);
}

}