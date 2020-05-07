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
// #ifndef _TAH_assert_h_
// #define _TAH_assert_h_

#undef assert

#ifdef TAA_NDEBUG

#define assert(e) noop
#define _line noop
#define _linef noop
#define _printf noop
#define PAUSE(n) noop
#define PAUSE_KEY(n) noop
#define PAUSE_KEYS noop

#else

#define assert(e) ((e) ? noop : assert_definition(#e, __FILE__, __LINE__))
// #define _line (assert_line(__FILE__, __LINE__))
// #define _linef (assert_line(__FILE__, __LINE__))
// #define _printf assert_line_reg(__FILE__, __LINE__), assert_print

#endif

#include <Common/Config.h>

namespace tas
{

TAA_LIB void assert_mode(uint _mode);
TAA_LIB void assert_definition(const char* expression, const char* file, uint line);
TAA_LIB void assert_line(const char* file, uint line);
TAA_LIB void assert_dump_file(char* file);
TAA_LIB void assert_print(char* fmt, ...);
TAA_LIB void assert_line_reg(const char* file, uint line);
TAA_LIB void* assert_file();

}

// #endif