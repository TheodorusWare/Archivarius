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
#ifndef _TAH_TemplateAllocator_h_
#define _TAH_TemplateAllocator_h_

#include <Common/Config.h>
// #include <typeinfo>

namespace tas
{

/** Allocator class for Array<>. */
template<class T>
class Allocator
{
public:
	/** Allocate memory for n objects.
	  * @param n Count object T.
	  * @return Allocated memory block.
	  */
	T* allocate(uint n)
	{
		// _printf("Allocator::allocate %u %s\n", n, typeid(T).name());
		return new T[n];
	}

	/** Deallocate memory.
	  * @param p Pointer from allocate().
	  */
	void deallocate(T* p)
	{
		if(!p) return;
		// _printf("Allocator::deallocate %X %s\n", p, typeid(T).name());
		safe_delete_array(p);
	}
};

/** Allocator class for Array<> objects. */
template<class T>
class AllocatorObj
{
public:
	/** Allocate memory for n objects.
	  * @param n Count object T.
	  * @return Allocated memory block.
	  */
	T* allocate(uint n)
	{
		return new T[n];
	}

	/** Deallocate memory.
	  * @param p Pointer from allocate().
	  */
	void deallocate(T* p)
	{
		if(!p) return;
		safe_delete_ao(p);
	}
};

}

#endif