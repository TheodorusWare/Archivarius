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
#include <Common/ConvertBytes.h>
#include <Common/String.h>

namespace tas
{

uint convertBytesToDecimal(wint& bytes, uint& rem)
{
	uint n = 0;
	rem = 0;
	for(; n < 3 and bytes >= 1024; n++)
	{
		rem = bytes % 1024;
		bytes /= 1024;
	}
	return n;
}

void convertDecimalSpace(String* valuesw, wint value)
{
	String& values = *valuesw;
	values.format("%llu", value);
	byte len = values.length();
	byte spaces = (len - 1) / 3;
	if(spaces)
	{
		char buffer[128] = {0};
		buffer[0] = 0;
		byte lenw = len + spaces;
		byte wc = 0;
		byte ri = len - 1;
		byte wi = lenw - 1;
		forn(len)
		{
			if(wc == 3)
			{
				buffer[wi--] = ' ';
				wc = 0;
			}
			wc++;
			buffer[wi--] = values[ri--];
		}
		buffer[lenw] = 0;
		values = buffer;
	}
}

}