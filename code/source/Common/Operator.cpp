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
#ifdef TAA_ALLOCATOR

#include <Common/platform/system.h>
#include <Common/platform/api.h>
#include <Common/platform/types.h>
#include <Common/platform/print.h>
#include <Common/platform/cpp.h>
#include <Common/Allocator.h>

#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#endif

// Overloading operators new, delete.
// Include this module if using Allocator.

namespace tas
{
TAA_LIB void* operator_new(uint_t size);
TAA_LIB void  operator_delete(void* p);
}

void* operator new(uint_t size)
{
	return tas::operator_new(size);
}

void* operator new[](uint_t size)
{
	return tas::operator_new(size);
}

// placement new
void* operator new (uint_t size, void* mem)
{
	return mem;
}

void* operator new[](uint_t size, void* mem)
{
	return mem;
}


void operator delete(void* p)
{
	tas::operator_delete(p);
}

void operator delete[](void* p)
{
	tas::operator_delete(p);
}

namespace tas
{

void* MemoryAllocClient(uint size)
{
#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	return HeapAlloc(GetProcessHeap(), 0, size);
#else
	return malloc(size);
#endif
}

void MemoryFreeClient(void* p)
{
#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	HeapFree(GetProcessHeap(), 0, p);
#else
	free(p);
#endif
}

void allocatorSetClientMemoryRoutines()
{
	allocatorSetMemoryRoutines(MemoryAllocClient, MemoryFreeClient);
}

} /// tas

#endif