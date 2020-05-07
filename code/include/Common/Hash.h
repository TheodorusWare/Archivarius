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
#ifndef _TAH_Hash_h_
#define _TAH_Hash_h_

#include <Common/Config.h>

namespace tas
{

TAA_LIB uint HashLy       (byte* str, uint len, uint hash = 0);
TAA_LIB uint HashDjb      (byte* str, uint len);
TAA_LIB uint HashMurmur2  (byte* key, uint len, uint seed);
TAA_LIB uint HashMurmur2A (byte* key, uint len, uint seed);

}

#endif