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
#include <Common/Bitwise.h>
#include <Common/Math.h>

namespace tas
{

byte bitGreat(uint value)
{
	if(value < 2) return 1;
	if(value > 0xFFFFFF)
		return log2i((float)(value >> 22)) + 23;
	return log2i((float)value) + 1;
}

uint bit_check(uint value, byte pos, byte bitn)
{
	return ((value & (1 << ((bitn - 1) - pos))) != 0);
}

uint bit_set(uint value, byte pos, bool state, byte bitn)
{
	if(state)
		return (value | (1 << ((bitn - 1) - pos)));
	else
		return (value & ~(1 << ((bitn - 1) - pos)));
}

uint bit_switch(uint value, byte pos, byte bitn)
{
	return (value ^ (1 << ((bitn - 1) - pos)));
}

byte bit_great(uint value, byte bitn)
{
	if(value == 0 || bitn == 0)
		return 0;

	forn(bitn)
	{
		if(value >> (bitn - i - 1))
			return bitn - i;
	}
}

/// mask[31] 2 4 8 .. ; maximum 5 iterations
byte bit_great_fast(uint value, uint* mask)
{
	if(value == 0)
		return 0;

	int mid = 0;
	int left = 0;
	int right = 30;

	while(left <= right)
	{
		mid = (left + right) / 2;
		if(value == mask[mid])
			return mid + 2;
		else if(value < mask[mid])
			right = mid - 1;
		else
			left = mid + 1;
	}

	if(right < mid)
		return mid + 1;
	else
		return mid + 2;

	return 0;
}

/// mask 0 1 2 4 8 ..
byte bit_great_mask(uint value, byte bitn, uint* mask)
{
	if(value == 0 || bitn == 0)
		return 0;

	forn(bitn)
	{
		if(value & mask[bitn - i])
			return bitn - i;
	}
}

byte bit_low(uint value, byte bitn)
{
	if(value == 0 || bitn == 0)
		return 0;

	forn(bitn)
	{
		uint sv = 1 << i;
		if(value & sv)
			return i + 1;
	}
}

void bit_print(uint value, byte nc, bool nl, byte bitn)
{
	forn(nc)
	{
		_printf("%d", bit_check(value, i, bitn));
		if((i+1)%4 == 0)
			_printf(" ");
	}
	if(nl)
		_printf("\n");
}

}