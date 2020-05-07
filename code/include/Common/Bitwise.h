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
#ifndef _TAH_Bitwise_h_
#define _TAH_Bitwise_h_

#include <Common/Config.h>

namespace tas
{

/** Bitwise operators
  * @param value Input.
  * @param pos Bit 0..31 from left to right.
  * @param state 1 or 0.
  * @param bitn Bit count in value.
  * @return Changed value.
  */
byte bitGreat(uint value);
uint bit_check(uint value, byte pos, byte bitn = 8);
uint bit_set(uint value, byte pos, bool state, byte bitn = 8);
uint bit_switch(uint value, byte pos, byte bitn = 8);
byte bit_great(uint value, byte bitn = 8);
byte bit_great_fast(uint value, uint* mask);
byte bit_great_mask(uint value, byte bitn, uint* mask);
byte bit_low(uint value, byte bitn);
void bit_print(uint value, byte nc = 8, bool nl = 1, byte bitn = 8);

}

#endif