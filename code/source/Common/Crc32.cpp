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
#include <Common/Crc32.h>

namespace tas
{

Crc32::Crc32()
{
	crc = 0xFFFFFFFF;
	table = 0;
}

Crc32::~Crc32()
{
	safe_delete_array(table);
}

void Crc32::tableInit()
{
	table = new uint[256];
	uint n, i, j;
	for(i = 0; i < 256; i++)
	{
		n = i;
		for(j = 0; j < 8; j++)
			n = n & 1 ? (n >> 1) ^ 0xEDB88320 : n >> 1;
		table[i] = n;
	}
}

void Crc32::calculate(byte* buf, uint len)
{
	while(len--)
		crc = table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
}

uint Crc32::get()
{
	return crc ^ 0xFFFFFFFF;
}

void Crc32::reset()
{
	crc = 0xFFFFFFFF;
}

}