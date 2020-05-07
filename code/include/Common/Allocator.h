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
/*****************************************************************
 *                                                               *
 *  Simple memory allocator                                      *
 *  This module redefinition standart operators new, delete      *
 *  for get extension information when calls this operators,     *
 *  such as file name, line number. For search memory leaks.     *
 *                                                               *
 *  alloc mode bit flags:                                        *
 *  0x01 - text log for print                                    *
 *  0x02 - keep pointer data size [size][data]                   *
 *  0x04 - memory alignment                                      *
 *  0x08 - memory pages for alloc, free                          *
 *  0x10 - winapi system alloc, free                             *
 *                                                               *
 *  alloc table mode bit flags:                                  *
 *  0x01 - build list for single file ALLOC_FILE                 *
 *  0x02 - disable clear list                                    *
 *  0x04 - decrease sort list by alloc size                      *
 *  0x08 - decrease sort list by alloc count                     *
 *  0x10 - insert in list unique items by file name, line        *
 *  0x20 - insert in list unique items by file name              *
 *  0x40 - alloc info list for search memory leaks               *
 *                                                               *
 *  alloc mode:                                                  *
 *  0x09 - default, mem pages, text log                          *
 *                                                               *
 *  alloc table mode:                                            *
 *  0x40 - info list for detect memory leak                      *
 *                                                               *
 *  note: ALLOCATOR_MODE in Alloc.h, bit flags                   *
 *  If you want use this allocator define TAA_ALLOCATOR          *
 *                                                               *
 *****************************************************************/

#ifndef _TAH_Allocator_h_
#define _TAH_Allocator_h_

/** @defgroup memalloc Memory allocator
    @{
*/

#include <Common/platform/system.h>
#include <Common/platform/api.h>
#include <Common/platform/types.h>
#include <Common/platform/badalloc.h>

namespace tas
{

/// Memory routine functions.
typedef void* (*MemoryAlloc) (uint size);
typedef void  (*MemoryFree)  (void* p);

#ifdef TAA_ALLOCATOR

/// Alloc mode bit flags.
#define ALLOCATOR_MODE 0x1D
#define ALLOCATOR_ALIGN 4

TAA_LIB void allocatorInit();
TAA_LIB void allocatorClean();
TAA_LIB void allocatorInfo();
TAA_LIB void allocatorSetMemorySize(uint page_size, uint chunk_size);
TAA_LIB void allocatorGetMemorySize(char* page_size, char m = 0);
TAA_LIB void allocatorPrintf(char* format, ...);
TAA_LIB void allocatorSetLog(wchar_t* name);
TAA_LIB void allocatorSetTableMode(uint mode);
TAA_LIB void allocatorSetMemoryRoutines(MemoryAlloc memAlloc, MemoryFree memFree);

void allocatorSetClientMemoryRoutines();

#else

/** Intializing memory allocator.
  */
inline void allocatorInit() {}

/** Set memory page size in bytes.
  * @param page_size Memory page size.
  * @param chunk_size Memory page pool chunk size.
  */
inline void allocatorSetMemorySize(uint page_size, uint chunk_size) {}

/** Get memory page, block size in string.
  * @param[out] page_size Memory page size string.
  * @param[out] block_size Memory block size string.
  * @param[in] m Type size
  * 	0: page size
  * 	1: alloc size current.
  */
inline void allocatorGetMemorySize(char* page_size, char m = 0) {}

/** Free memory. */
inline void allocatorClean() {}

/** Save alloc information to log. */
inline void allocatorInfo() {}

/** Print message to alloc log.
  * @param format Formated string.
  */
inline void allocatorPrintf(char* format, ...) {}

/** Set log file name.
  * @param name Log file.
  */
inline void allocatorSetLog(char* name) {}

/** Set table mode.
  * @param mode Mode.
  * @remark Call after allocatorInit().
  */
inline void allocatorSetTableMode(uint mode) {}

/** Set memory routine functions.
  * @param memAlloc Routine for alloc memory.
  * @param memFree Routine for free memory.
  * @remark This routines should use malloc, free (unix, windows), VirtualAlloc, VirtualFree (windows),
  * 		not operators new, delete. Call before allocatorInit().
  */
inline void allocatorSetMemoryRoutines(MemoryAlloc memAlloc, MemoryFree memFree) {}

/** Set client memory routine functions.
  * Implemented in Operator.cpp.
  */
inline void allocatorSetClientMemoryRoutines() {}

#endif

} /// namespace

/** @} */
#endif