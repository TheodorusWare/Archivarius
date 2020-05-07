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
#include <Common/Hash.h>

namespace tas
{

uint HashLy(byte* str, uint len, uint hash)
{
	forn(len)
	hash = (hash * 1664525) + str[i] + 1013904223;
	return hash;
}

uint HashDjb(byte* str, uint len)
{
	uint hash = 5381;
	forn(len)
	hash = ((hash << 5) + hash) + str[i];
	return hash;
}

uint HashMurmur2(byte* key, uint len, uint seed)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	uint h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const byte* data = (const byte*) key;

	while(len >= 4)
	{
		uint k = *(uint *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3:
		h ^= data[2] << 16;
	case 2:
		h ^= data[1] << 8;
	case 1:
		h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

uint HashMurmur2A(byte* key, uint len, uint seed)
{
	const uint m = 0x5bd1e995;
	const int r = 24;
	uint l = len;

	const byte* data = (const byte*) key;

	uint h = seed;

	while(len >= 4)
	{
		uint k = *(uint*)data;

		mmix(h,k);

		data += 4;
		len -= 4;
	}

	uint t = 0;

	switch(len)
	{
	case 3:
		t ^= data[2] << 16;
	case 2:
		t ^= data[1] << 8;
	case 1:
		t ^= data[0];
	};

	mmix(h,t);
	mmix(h,l);

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

}