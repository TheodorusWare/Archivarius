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
#ifndef _platform_types_h_
#define _platform_types_h_

typedef char      int8,  b8;
typedef short     int16, b16;
typedef int       int32, b32;
typedef long long int64, b64;

typedef unsigned char      uint8,  u8,  byte;
typedef unsigned short     uint16, u16, half, word;
typedef unsigned int       uint32, u32, uint;
typedef unsigned long long uint64, u64, wint;

#if TAA_ARCHITECTURE == TAA_ARCHITECTURE_64
typedef wint uint_t;
#else
typedef uint uint_t;
#endif

typedef float  f32;
typedef double f64;

typedef char* chars;

#define signed_type(type)   ((type) -1 < 0)
#define unsigned_type(type) ((type) -1 > 0)

#define MIN_INT8   0x80
#define MIN_INT16  0x8000
#define MIN_INT32  0x80000000
#define MAX_INT8   0x7F
#define MAX_INT16  0x7FFF
#define MAX_INT32  0x7FFFFFFF
#define MAX_UINT8  0xFF
#define MAX_UINT16 0xFFFF
#define MAX_UINT32 0xFFFFFFFF

#define KB 0x400
#define MB 0x100000
#define GB 0x40000000

#endif