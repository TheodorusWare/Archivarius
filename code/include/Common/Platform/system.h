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
#ifndef _TAH_platform_system_h_
#define _TAH_platform_system_h_

// TAA_PLATFORM
#define TAA_PLATFORM_WINDOWS 1
#define TAA_PLATFORM_LINUX 2
#define TAA_PLATFORM_UNKNOWN 3

// TAA_PLATFORM_TYPE
#define TAA_PLATFORM_DESKTOP 1
#define TAA_PLATFORM_MOBILE 2

// TAA_COMPILER
#define TAA_COMPILER_MSVC 1
#define TAA_COMPILER_GNUC 2

// TAA_ARCHITECTURE
#define TAA_ARCHITECTURE_32 1
#define TAA_ARCHITECTURE_64 2

#if defined(_MSC_VER)
#define TAA_COMPILER TAA_COMPILER_MSVC
#define TAA_COMP_VER _MSC_VER
#define TAA_IDE_VER (8 + ((TAA_COMP_VER - 1400) / 100))
#define TAA_COMP_NAME "MSVC"
#define __FUNC__ __FUNCTION__
#elif defined(__GNUC__)
#define TAA_COMPILER TAA_COMPILER_GNUC
#define TAA_COMP_VER (((__GNUC__)*100) + (__GNUC_MINOR__*10) + __GNUC_PATCHLEVEL__)
#define TAA_COMP_NAME "GNUC"
#define __FUNC__ __PRETTY_FUNCTION__
#else
#pragma error "No known compiler."
#error "No known compiler."
#endif

#define TAA_IDE "GNU Makefile"

#if defined(__WIN32__) || defined(_WIN32)
#define TAA_PLATFORM TAA_PLATFORM_WINDOWS
#define TAA_PLATFORM_TYPE TAA_PLATFORM_DESKTOP
#elif defined(__unix__)
#define TAA_PLATFORM TAA_PLATFORM_LINUX
#ifdef __ANDROID__
#define TAA_PLATFORM_TYPE TAA_PLATFORM_MOBILE
#else
#define TAA_PLATFORM_TYPE TAA_PLATFORM_DESKTOP
#endif
#else
#define TAA_PLATFORM TAA_PLATFORM_UNKNOWN
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__) || defined(__alpha__) || defined(__ia64__) || defined(__s390__) || defined(__s390x__)
#define TAA_ARCHITECTURE TAA_ARCHITECTURE_64
#else
#define TAA_ARCHITECTURE TAA_ARCHITECTURE_32
#endif

#if TAA_COMPILER == TAA_COMPILER_MSVC
#if TAA_COMP_VER >= 1400
#define TAA_SSE
#define TAA_INLINE __forceinline
#else
#define TAA_INLINE inline
#endif
#else
#define TAA_INLINE __inline
#endif

#if TAA_COMPILER == TAA_COMPILER_MSVC && TAA_COMP_VER >= 1600
#define TAA_VSNPRINTF
#elif TAA_COMPILER == TAA_COMPILER_GNUC && TAA_COMP_VER >= 310
#define TAA_VSNPRINTF
#endif

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <iso646.h>
#define PATH_SEP '\\'
#define PATH_SEP_UP "..\\"
#define PATH_SEP_REV '/'
#define TIMEMS (clock())
#pragma warning (disable : 4577)
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#define PATH_SEP '/'
#define PATH_SEP_UP "../"
#define PATH_SEP_REV '\\'
#define TIMEMS (clock()/1000)
#endif

#define PAUSE(n) {uint start = TIMEMS; while(TIMEMS - start < n);}
#define PAUSE_KEY(k) while(!HIWORD(GetAsyncKeyState(k)))
#define PAUSE_KEYS PAUSE_KEY(VK_SPACE) KEY_STOP; PAUSE(300)
#define KEY_STATE(k) ((GetAsyncKeyState(k) & 0x8000) != 0)
#define KEY_STOP assert(!KEY_STATE(VK_ESCAPE))

// #error message
// #pragma message("message")

#endif