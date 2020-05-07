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
#ifndef _TAH_AllocatorConfig_h_
#define _TAH_AllocatorConfig_h_

/** @addtogroup memalloc
    @{
*/

#if defined(TAA_ALLOCATOR) && !defined(TAA_ALLOCATOR_SRC)

#include <Common/platform/types.h>

/** Placement new.
  * @param size Size of data.
  * @param mem Allocated memory.
  * @remark new(mem) obj.
  */
void* operator new   (uint_t size, void* mem);
void* operator new[] (uint_t size, void* mem);

/// Say thank you for this definitions Ogre3D
#define new (allocatorSrc(__FILE__,__LINE__,0,0),0) ? 0 : new
#define newa(n) (allocatorAlign(n),0) ? 0 : new
/// #define delete (allocatorSrc(__FILE__,__LINE__,1),0)   ? noop : delete

/// Return pointer data size.
#define ALLOCATOR_SIZE(p) (((uint*)p)[-1])

namespace tas
{

/** Set file, line source, when call operators new, delete.
  * @param file Source file.
  * @param line Source line.
  * @param mode Allocate by 0, Deallocate by 1.
  * @param ptr Delete pointer.
  */
TAA_LIB void allocatorSrc(const char* file, uint line, byte mode, uint ptr);

/** Set align size before once allocation memory.
  * @param align Size.
  */
TAA_LIB void allocatorAlign(uint align);

/** Check pointer valid.
  * @param p Pointer.
  * @param size Pointer data.
  * @return Pointer valid or invalid.
  */
TAA_LIB bool allocatorValid(void* p, uint size);

/** Get pointer data size.
  * @param p Pointer.
  * @return Pointer size or null.
  */
TAA_LIB uint allocatorSize(void* p);

}

#endif
/** @} */
#endif