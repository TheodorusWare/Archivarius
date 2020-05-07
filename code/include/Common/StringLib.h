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
#ifndef _TAH_StringLib_h_
#define _TAH_StringLib_h_

#include <Common/Config.h>

// std*  : direct access
// stm*  : memory placement
// *cpy  : copy
// *cmp  : compare
// *cmpi : compare with relationship
// *cat  : concatenate
// *chr  : search char
// *rchr : search reverse
// *str  : search string
// *pat  : search pattern
// *ins  : insert sequence
// *gen  : generate sequence

namespace tas
{

TAA_LIB char* stdcpy  (char* dst, char* src);
TAA_LIB wchar_t* stwcpy  (wchar_t* dst, wchar_t* src);
TAA_LIB char* stdncpy (char* dst, char* src, uint n);
TAA_LIB char* stdcpyf (char* dst, char* fmt, ...);
TAA_LIB char* stdcat  (char* dst, char* src);
TAA_LIB char* stdncat (char* dst, char* src, uint n);
TAA_LIB bool  stdcmp  (char* dst, char* src, bool noCase = 0);
TAA_LIB bool  stdncmp (char* dst, char* src, uint n, bool noCase = 0);
TAA_LIB int   stdcmpi (char* dst, char* src, bool noCase = 0);
TAA_LIB int   stdncmpi(char* dst, char* src, uint n, bool noCase = 0);
TAA_LIB bool  stdpat  (char* src, char* pat);
TAA_LIB bool  stwpat  (wchar_t* src, wchar_t* pat);
TAA_LIB uint  stdlen  (char* src);
TAA_LIB uint  stwlen  (wchar_t* src);
TAA_LIB int   stdchr  (char* src, char c);
TAA_LIB int   stdrchr (char* src, char c);
TAA_LIB int   stdstr  (char* src, char* pat);
TAA_LIB int   stdrstr (char* src, char* pat);
TAA_LIB char* stdins  (char* dst, char c, uint p);
TAA_LIB char* stdins  (char* dst, char* ins, uint p);
TAA_LIB char* stdgen  (char* dst, uint ns, uint nd, char uq, uint rv, char sp, bool lc);
TAA_LIB wchar_t* stwgen  (wchar_t* dst, uint ns, uint nd, wchar_t uq, uint rv, wchar_t sp, bool lc);
TAA_LIB char* stdext  (char* dst, char* src, char sp, char rv, char m);
TAA_LIB char* stdlwc  (char* src);
TAA_LIB char  stdlwc  (char  c);
TAA_LIB char* stdupc  (char* src);
TAA_LIB char  stdupc  (char  c);
TAA_LIB char* stdtime (char* dst, char type);
TAA_LIB char* stditoa (char* src, int val, int base);
TAA_LIB char* stdftoa (char* src, float val, int prec);
TAA_LIB int   stdatoi (char* src);
TAA_LIB int   stdwtoi (wchar_t* src);
TAA_LIB float stdatof (char* src);

TAA_LIB void  stdprintf  (char* fmt, ...);
TAA_LIB void  stdfprintf (char* file, char* fmt, ...);

TAA_LIB char* stmcpy  (chars& dst, char* src);
TAA_LIB char* stmncpy (chars& dst, char* src, uint n);
TAA_LIB char* stmcpyf (chars& dst, char* fmt, ...);
TAA_LIB char* stmcat  (chars& dst, char* src);
TAA_LIB char* stmncat (chars& dst, char* src, uint n);
TAA_LIB char* stmins  (chars& src, char* ins, uint p);
TAA_LIB char* stmgen  (chars& dst, uint ns, uint nd, char uq, uint rv, char sp, bool lc);
TAA_LIB char* stmext  (chars& dst, char* src, char sp, char rv, char m);

TAA_LIB char* stmcpy  (char* src);
TAA_LIB char* stmncpy (char* src, uint n);
TAA_LIB char* stmgen  (uint ns, uint nd, char uq, uint rv, char sp, bool lc);

TAA_LIB void* mencpy  (void* dst, void* src, uint n);
TAA_LIB void* menmove (void* dst, void* src, uint n);
TAA_LIB void* menset  (void* dst, int value, uint n);

/** Allocate memory block and keep size.
  * @param size Block size.
  * @return Allocated memory block.
  * @remark Memory structure [size][data].
  * This method equivalent method from stdlib.h
  */
TAA_LIB void* stmalloc(uint size);

/** Free memory block.
  * @param ptr Pointer allocated from pmalloc() or prealloc().
  * @remark This method equivalent method from stdlib.h
  */
TAA_LIB void stmfree(void* ptr);

/** Retrieve pointer data size.
  * @param ptr Pointer allocated from pmalloc() or prealloc().
  */
TAA_LIB uint stmsize(void* ptr);

/** Reallocate memory block.
  * @param ptr Pointer allocated from malloc() or realloc().
  * @param size New block size.
  * @return Current block or new allocated memory block.
  * @remark This method equivalent method from stdlib.h
  */
TAA_LIB void* stmrealloc(void* ptr, uint size);

/** Reallocate memory block.
  * @param ptr Pointer allocated from new[].
  * @param csize Current block size.
  * @param nsize New block size.
  * @return Current block or new allocated memory block.
  * @remark Pointer should not be
  * allocated from malloc() or realloc(p, sz).
  */
TAA_LIB void* stmrealloc(void* ptr, uint csize, uint nsize);

}

#endif