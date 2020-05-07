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

#define TAA_ALLOCATOR_SRC
#include <Common/Allocator.h>
#undef TAA_ALLOCATOR

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <Common/platform/cpp.h>
#include <Common/platform/system.h>
#include <Common/platform/badalloc.h>
#include <Common/platform/assert.h>
#include <Common/container/Array.h>
#include <Common/StringLib.h>

#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#endif

#define PATH_SEP '/'

#define _printf
#define _line

/// placement
void* operator new(uint_t size, void* mem);
void* operator new[](uint_t size, void* mem);

namespace tas
{

TAA_LIB void allocatorSrc(const char* file, uint line, byte mode, uint ptr);
TAA_LIB void allocatorAlign(uint align);
TAA_LIB bool allocatorValid(void* p, uint size);
TAA_LIB uint allocatorSize(void* p);
TAA_LIB void* operator_new(uint_t size);
TAA_LIB void  operator_delete(void* p);

#define ALLOCATOR_MODE_TABLE alc->allocatorTableMode

#define ALLOC_FILE "LzreCoder.cpp"

#define PSIZE(p) *((uint*)p)
#define PSIZEH(p) *((uint*)(p-sizeof(uint)))

struct AllocInfo
{
	uint_t address;
	uint size;
	char* file;
	uint line;
	uint allocCount;
};

// user memory allocate, free routines
static MemoryAlloc userMemoryAlloc = 0;
static MemoryFree  userMemoryFree = 0;

void* SysAlloc(uint size);
void  SysFree(void* p);

template<class T>
class AllocSys
{
public:
	T* allocate(uint n)
	{
		// _printf("allocate %u %u %u \n", sizeof(T), n, sizeof(T) * n);
		return (T*) SysAlloc(sizeof(T) * n);
	}
	void deallocate(T* p)
	{
		return SysFree(p);
	}
};

void _conv_bytes_s(char* so, uint bytes, uint prec, byte width = 0);

#if ALLOCATOR_MODE & 0x08
#include "MemoryPage.hpp"
#endif

struct AllocStor
{
#if ALLOCATOR_MODE & 0x08
	MemoryPage* memPage;
#endif

	uint allocatorTableMode;

	uint allocCount;
	uint allocSize;
	uint allocSizeMax;
	uint freeCount;
	uint freeSize;

	char* file; // allocatorSrc__() data
	uint  line;
	uint  alignSize;

	Array<AllocInfo*, AllocSys<AllocInfo*> > list;
#if TAA_PLATFORM == TAA_PLATFORM_LINUX
	Array<uint, AllocSys<uint> > stack;
#endif

	FILE* fileLog;
	wchar_t* fileName;

	AllocStor()
	{
#if ALLOCATOR_MODE & 0x08
		memPage = 0;
#endif
		allocatorTableMode = 0;
		allocCount = 0;
		allocSize = 0;
		allocSizeMax = 0;
		freeCount = 0;
		freeSize = 0;
		file = 0;
		line = 0;
		alignSize = 0;
		fileLog = 0;
		fileName = 0;
#if TAA_PLATFORM == TAA_PLATFORM_LINUX
		stack.reserve(16);
#endif
	}
};

static AllocStor* alc = 0;

static void _push_alloc_list(void* p, uint size);
static bool _erase_alloc_list(void* p);

struct MemLeak
{
	char file[50];
	uint line;
	uint totalAlloc;
	uint totalSize;
};

MemLeak* _build_list(uint& mln);

void* SysAlloc(uint size)
{
	if(userMemoryAlloc)
		return userMemoryAlloc(size);
	else
#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		// return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
		return HeapAlloc(GetProcessHeap(), 0, size);
#else
		return malloc(size);
#endif
}

void SysFree(void* p)
{
	if(!p) return;
	if(userMemoryFree)
		userMemoryFree(p);
	else
#if ALLOCATOR_MODE & 0x10 and TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		// VirtualFree(p, 0, MEM_RELEASE);
		HeapFree(GetProcessHeap(), 0, p);
#else
		free(p);
#endif
}

void allocatorSetMemoryRoutines(MemoryAlloc memAllocl, MemoryFree memFreel)
{
	userMemoryAlloc = memAllocl;
	userMemoryFree = memFreel;
}

void* memAlloc(uint size)
{
#if ALLOCATOR_MODE & 0x08
	assert(alc);
	assert(alc->memPage);
	// _printu(alc->alignSize);
	// _printu(size);
	void* mp = MemoryPageAlloc(alc->memPage, size, alc->alignSize);
	alc->alignSize = 0;
	return mp;
#else
	return SysAlloc(size);
#endif
}

void memFree(void* p)
{
	if(!p) return;
#if ALLOCATOR_MODE & 0x08
#if ALLOCATOR_MODE & 0x02
	PSIZE(p) = 0;
#endif
	MemoryPageFree(alc->memPage, p);
#else
	SysFree(p);
#endif
}

void* operator_new(uint_t size)
{
	assert(alc);
	// _printu(size);

	void* p = 0;

	if(size == 0)
		size = 1;
#if TAA_PLATFORM == TAA_PLATFORM_LINUX
	if(!alc->file)
	{
		p = SysAlloc(size);
		if(p == 0)
			throw BadAlloc();
		return p;
	}
#endif
	uint usize = size;

	// if(ALLOCATOR_MODE_TABLE & 0x40)
	// usize += sizeof(uint_t);

#if ALLOCATOR_MODE & 0x02
	usize += sizeof(uint);
	p = memAlloc(usize);
	if(p)
	{
		char* pc = (char*) p;
		PSIZE(pc) = size;
		pc += sizeof(uint);
		p = pc;
	}
#else
	p = memAlloc(usize);
#endif

	if(p == 0)
		throw BadAlloc();

	alc->allocCount++;
	alc->allocSize += size;
	if(alc->allocSize > alc->allocSizeMax)
		alc->allocSizeMax = alc->allocSize;

	if(ALLOCATOR_MODE_TABLE & 0x40)
	{
		_push_alloc_list(p, size);
		// p += 4;
		// p = (uint_t*) p + 1;
	}

	alc->file = 0;
	return p;
}

void operator_delete(void* p)
{
	if(!alc or !p) return;

#if TAA_PLATFORM == TAA_PLATFORM_LINUX
	if(!alc->stack.size() || alc->stack.last() != (uint) p)
	{
		SysFree(p);
		return;
	}
	alc->stack.pop();
#endif

	// alc->freeCount++;

	if((ALLOCATOR_MODE_TABLE & 0x42) == 0x40)
	{
		if(_erase_alloc_list(p))
			alc->freeCount++;
		// p = (uint*) p - 1;
	}
	else
		alc->freeCount++;

#if ALLOCATOR_MODE & 0x02
	if(alc->file)
		alc->allocSize -= PSIZEH(p);
	memFree((char*)p-sizeof(uint));
#else
	memFree(p);
#endif

	alc->file = 0;
}

void allocatorInit()
{
	uint msz = 1024;
	char* mem = (char*) SysAlloc(msz);
	char* sme = mem;

	alc = new(mem) AllocStor;
	mem += sizeof(AllocStor);

#if ALLOCATOR_MODE & 0x08
	alc->memPage = MemoryPageCreate(mem);
	mem += sizeof(MemoryPage) + 10;
#endif
	alc->fileName = reinterpret_cast<wchar_t*>(mem);
	stwcpy(alc->fileName, L"allocator.log");
}

bool allocatorValid(void* p, uint size)
{
	if(!p) return 0;
#if ALLOCATOR_MODE & 0x08
#if ALLOCATOR_MODE & 0x02
	return MemoryPageValid(alc->memPage, (char*)p-sizeof(uint), size);
#else
	return 0;
#endif
#endif
	return 0;
}

uint allocatorSize(void* p)
{
	if(!p) return 0;

#if ALLOCATOR_MODE & 0x02
	return PSIZEH((char*)p);
#endif

	if(!(ALLOCATOR_MODE_TABLE & 0x40))
		return 0;

	forn(alc->list.size())
	if(alc->list[i]->address == (uint_t) p)
		return alc->list[i]->size;

	return 0;
}

void allocatorSrc(const char* file, uint line, byte mode, uint ptr)
{
	// _printf("SRC | %u | %u | %s\n", mode, line, _file_name(file));
	assert(alc && "call alloc_init() before call operators new ..");
	alc->file = (char*)file;
	alc->line = line;
	// append delete pointer to stack
#if TAA_PLATFORM == TAA_PLATFORM_LINUX
	if(mode) alc->stack.push_back(ptr);
#endif
}

void allocatorAlign(uint align)
{
	alc->alignSize = align;
}

void allocatorSetLog(wchar_t* name)
{
	stwcpy(alc->fileName, name);
}

void allocatorSetTableMode(uint mode)
{
	alc->allocatorTableMode = mode;
	if(ALLOCATOR_MODE_TABLE & 0x40)
		alc->list.reserve(16);
}

void allocatorPrintf(char* format, ...)
{
	static char buffer[512] = {0};
	va_list args;
	va_start (args, format);
	vsprintf (buffer, format, args);
	va_end (args);

#if ALLOCATOR_MODE & 0x01
	if(!alc->fileLog)
		alc->fileLog = _wfopen(alc->fileName, L"w");
	fprintf(alc->fileLog, buffer);
	_wfreopen(alc->fileName, L"a", alc->fileLog);
#else
	stdprintf(buffer);
#endif
}

void allocatorSetMemorySize(uint page_size, uint chunk_size)
{
#if ALLOCATOR_MODE & 0x08
	MemoryPageSetSize(alc->memPage, page_size, chunk_size);
#endif
}

void allocatorGetMemorySize(char* page_size, char m)
{
#if ALLOCATOR_MODE & 0x08
	MemoryPageGetSizes(alc->memPage, page_size, m);
#endif
}

void allocatorInfo()
{
	char cvs[3][16] = {0};

	_conv_bytes_s(cvs[0], alc->allocSizeMax, 1, 3);
	_conv_bytes_s(cvs[1], alc->freeSize, 1, 3);

#if ALLOCATOR_MODE & 0x01
	if(alc->fileLog)
		allocatorPrintf("\n");
#if ALLOCATOR_MODE & 0x08
	MemoryPageInfo(alc->memPage);
	allocatorPrintf("\n");
#endif
#if ALLOCATOR_MODE & 0x02
	allocatorPrintf("allocate query %u, size %s\n", alc->allocCount, cvs[0]);
#else
	allocatorPrintf("allocate query %u\n", alc->allocCount);
#endif
	int delMiss = alc->allocCount - alc->freeCount;
	allocatorPrintf("deletion query %u", alc->freeCount);
	if(delMiss != 0)
	{
		allocatorPrintf("\ndeletion %s %u", delMiss < 0 ? "wrong" : "missed", delMiss < 0 ? -delMiss : delMiss);
		stdprintf("\n======================================================\n");
		if(delMiss > 0)
			stdprintf(" Allocator: missing deletion of objects in count %u\n", delMiss);
		else
			stdprintf(" Allocator: incorrect deletion of objects in count %d\n", -delMiss);
		stdprintf("======================================================\n\n");
	}
#endif

	if(ALLOCATOR_MODE_TABLE & 0x40)
	{
		_line;
		if(alc->list.size())
		{
			_line;
			uint tablesz = 0;
			MemLeak* table = _build_list(tablesz);

			allocatorPrintf("\n\n|================================================================|\n");
			if(ALLOCATOR_MODE_TABLE & 0x02)
				allocatorPrintf("|                    Memory alloc table                          |\n");
			else
				allocatorPrintf("|                    Memory leak table                           |\n");

			allocatorPrintf("|================================================================|\n");
			if(ALLOCATOR_MODE_TABLE & 0x20)
				allocatorPrintf("| item | alloc | size   | file                                   |\n");
			else
				allocatorPrintf("| item | alloc | size   | line | file                            |\n");

			allocatorPrintf("|================================================================|\n");

			forn(tablesz)
			{
				_conv_bytes_s(cvs[2], table[i]. totalSize, 0);
				if(ALLOCATOR_MODE_TABLE & 0x20)
					allocatorPrintf("| %-4u | %-5u | %-6s | %-38s |\n",
					                i, table[i].totalAlloc, cvs[2], table[i]. file);
				else
					allocatorPrintf("| %-4u | %-5u | %-6s | %-4u | %-31s |\n",
					                i, table[i].totalAlloc, cvs[2], table[i].line, table[i].file);
			}

			allocatorPrintf("|================================================================|");

			memFree(table);

			/// clear list memory
			forn(alc->list.size())
			memFree(alc->list[i]);
			alc->list.clear();
#if TAA_PLATFORM == TAA_PLATFORM_LINUX
			alc->stack.clear();
#endif
		}
	}

	if(alc->fileLog)
	{
		fclose(alc->fileLog);
		alc->fileLog = 0;
	}
}

void allocatorClean()
{
#if ALLOCATOR_MODE & 0x08
	MemoryPageClear(alc->memPage);
#endif
	SysFree(alc);
	alc = 0;
}

void _push_alloc_list(void* p, uint size)
{
	int fs = stdrchr((char*) alc->file, PATH_SEP) + 1;

	if(ALLOCATOR_MODE_TABLE & 0x01)
		if(stdcmp(alc->file + fs, ALLOC_FILE))
			return;

	if(ALLOCATOR_MODE_TABLE & 0x30)
	{
		/// not push in list identity line, file
		forn(alc->list.size())
		{
			bool state = 0;
			if(ALLOCATOR_MODE_TABLE & 0x20)
				state = stdcmp(alc->list[i]->file, alc->file + fs);
			else if(ALLOCATOR_MODE_TABLE & 0x10)
				state = (stdcmp(alc->list[i]->file, alc->file + fs) and alc->list[i]->line == alc->line);
			if(state)
			{
				alc->list[i]->allocCount++;
				alc->list[i]->size += size;
				return;
			}
		}
	}

	uint len = stdlen((char*)alc->file) - fs + 2;
	uint memsz[3] = {sizeof(AllocInfo), len, 0};
	forn(2) memsz[2] += memsz[i];

	char* mem = (char*) memAlloc(memsz[2]);

	AllocInfo* nfo = (AllocInfo*) mem;
	nfo->address = (uint_t) p;
	nfo->size = size;
	nfo->line = alc->line;
	nfo->allocCount = 1;
	nfo->file = (char*) mem + memsz[0];
	stdcpy(nfo->file, (char*)alc->file + fs); // file name without path

	// *((uint*) p) = alc->list.size();

	alc->list.push_back(nfo);
}

bool _erase_alloc_list(void* p)
{
	int fs = 0;
	if(ALLOCATOR_MODE_TABLE & 0x21)
		fs = stdrchr((char*) alc->file, PATH_SEP) + 1;

	if(ALLOCATOR_MODE_TABLE & 0x01)
		if(stdcmp(alc->file + fs, ALLOC_FILE))
			return 1;

	/// restore index list
	// uint i = *((uint*) p - 1);
	// assert(i < alc->list.size());
	// if(i < alc->list.size())

	forn(alc->list.size())
	{
		if(ALLOCATOR_MODE_TABLE & 0x20)
		{
			if(stdcmp(alc->list[i]->file, alc->file + fs))
			{
				alc->list[i]->allocCount--;
				if(alc->list[i]->allocCount == 0)
				{
					memFree(alc->list[i]);
					alc->list.erase_swap(i);
				}
				return 1;
			}
		}
		else
		{
			if(alc->list[i]->address == (uint_t) p)
			{
				alc->freeSize += alc->list[i]->size;
				memFree(alc->list[i]);
				/// erase from array
				alc->list.erase_swap(i);
				return 1;
			}
		}
	}
	return 0;
}

MemLeak* _build_list(uint& mln)
{
	MemLeak* memLeak = (MemLeak*) memAlloc(sizeof(MemLeak) * alc->list.size());
	mln = 0;

	forn(alc->list.size())
	{
		bool find = 0;
		if((ALLOCATOR_MODE_TABLE & 0x30) == 0)
			form(u, mln)
		{
			if(stdcmp(memLeak[u].file, alc->list[i]->file) && memLeak[u].line == alc->list[i]->line)
			{
				memLeak[u].totalAlloc++;
				memLeak[u].totalSize += alc->list[i]->size;
				find = 1;
				break;
			}
		}
		if(!find)
		{
			stdcpy(memLeak[mln].file, alc->list[i]->file);
			memLeak[mln].line = alc->list[i]->line;
			memLeak[mln].totalAlloc = 1;
			memLeak[mln++].totalSize = alc->list[i]->size;
		}
	}

	if (mln == 1 or (ALLOCATOR_MODE_TABLE & 0x0C) == 0)
		return memLeak;

	/// sort
	forn(mln)
	{
		uint res = i;
		for(int u = i+1; u < mln; u++)
		{
			if(ALLOCATOR_MODE_TABLE & 0x04)
			{
				if(memLeak[res].totalSize < memLeak[u].totalSize)
					res = u;
			}
			else if(ALLOCATOR_MODE_TABLE & 0x08)
			{
				if(memLeak[res]. totalAlloc < memLeak[u]. totalAlloc)
					res = u;
			}
		}
		if(i != res)
		{
			MemLeak temp = memLeak[i];
			memLeak[i] = memLeak[res];
			memLeak[res] = temp;
		}
	}

	return memLeak;
}

// ret mode 0 - not conv, 1 - kb, 2 - mb
int _conv_bytes(uint bytes, float& osize)
{
	float kb = 1024.0;
	float mb = 1024.0 * 1024.0;
	float bytesf = bytes;
	int mode = 0;
	if (bytes >= mb)
	{
		osize = bytesf / mb;
		mode = 2;
	}
	else if (bytes >= kb)
	{
		osize = bytesf / kb;
		mode = 1;
	}
	else
	{
		osize = bytesf;
		mode = 0;
	}
	return mode;
}

void _conv_bytes_s(char* so, uint bytes, uint prec, byte width)
{
	char* postfix = "";
	float cvsize = 0;
	int m = _conv_bytes(bytes, cvsize);
	if (m == 1)
		postfix = "kb";
	if (m == 2)
		postfix = "mb";
	sprintf(so, "%-*.*f %s", width, prec, cvsize, postfix);
}

} /// namespace

#endif