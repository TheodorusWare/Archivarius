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
#ifndef _TAH_platform_print_h_
#define _TAH_platform_print_h_

// Debug printf routines
// #define _line  stdprintf("\n%-4u | _ _ _\n", __LINE__)
#define _line  stdprintf("\n%-4u | %s | _ _ _\n", __LINE__, _file_name(__FILE__))
#define _linef _line

#define _printi(x) _printf("%s = %d\n", #x, x)
#define _printu(x) _printf("%s = %u\n", #x, x)
#define _prints(x) _printf("%s = %s\n", #x, x)
#define _printp(x) _printf("%s = %X\n", #x, x)
#define _printc(x) _printf("%s = %c\n", #x, x)
#define _printr(x) _printf("%s = %.2f\n", #x, x)
#define _printv(v) _printf("%s = %.2f %.2f %.2f\n", #v, v.x, v.y, v.z);
#define _printsi(s,x) _printf("%s = %d\n", s, x)
#define _printsu(s,x) _printf("%s = %u\n", s, x)

#ifdef TAA_NDEBUG
#define _line noop
#define _linef noop
#define _printf noop
#define _fprintf noop
#else
// #define _printf (stdprintf("%-4u | ", __LINE__),0) ? noop : stdprintf
#define _printf (stdprintf("%-4u | %s | ", __LINE__, _file_name(__FILE__)),0) ? noop : stdprintf
#define _fprintf stdfprintf
#endif

#define _file_name(x)  (x + stdfname((char*)x) + 1)
#define _file_namer(x) (x + stdrchr((char*)x, '\\') + 1)
#define _type_test(type) _printf("type %s size %u signed %u\n", #type, sizeof(type), (type)-1 < 0);

// assertion
#define _asserte(e) ASSERTION(e)

namespace tas
{
TAA_LIB void stdprintf  (char* str,  ...);
TAA_LIB void stdfprintf (char* file, char* fmt, ...);
TAA_LIB int  stdrchr    (char* src,  char c);
TAA_LIB int  stdfname   (char* src);
}

#endif