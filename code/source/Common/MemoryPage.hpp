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
/**
	This *.hpp file included in Allocator.cpp

	Update 07.04.19
	Appended address header
	[size][.. data ..] <== new address
	[size][.. next avail block ..] <== free address
	size and next avail block is 4 byte
	next avail block is offset from start memory page
	to next block header
	This method decrease fragmentation of data.

	Update 29.11.19
	Removes address header.
	Implements memory pool for allocate small fixed size blocks.
	This method decrease fragmentation of data.
*/

byte alignSize(void* address, byte align);
struct MemoryPageBlock;
struct MemoryPagePool;

// MemoryPage allocator
struct MemoryPage
{
	uint pageSize;
	uint allocCount; // mpb
	uint freeCount;
	uint freePages;

	uint allocCountMax;
	uint allocSizeMax; // max bytes
	uint allocSizeCur;

	Array<MemoryPageBlock*, AllocSys<MemoryPageBlock*> > list;

	MemoryPageBlock* cmp; // cur mpage
	MemoryPageBlock* mpg;
	uint mpgi; // index

	MemoryPagePool* pool; // cur pool
	uint chunkSize; // single pool chunk size, const
	uint chunkCount; // count chunks in single pool, const
	uint chunkFree; // count free pool chunks
	uint freePools;

	MemoryPage()
	{
		pageSize = 65536;
		allocCount = 0;
		freeCount = 0;
		freePages = 0;
		allocCountMax = 0;
		allocSizeMax = 0;
		allocSizeCur = 0;
		cmp = 0;
		mpg = 0;
		mpgi = 0;
		pool = 0;
		chunkSize = 0;
		chunkCount = 0;
		chunkFree = 0;
		freePools = 0;
		list.reserve(16);
	}

	void* Alloc(uint size, uint align);
	void* AllocChunk(uint size, uint align);
	void  Free (void* p);
	void  FreeChunk (void* p);
};

#define MPB_STACK 20
struct MemoryPageBlock
{
	char* mem; // base
	uint  memUsage;
	uint pageSize;
	uint addressCount;
	uint_t lastAddr[MPB_STACK]; // stack ring buffer
	uint beg, end, cnt, siz; // ring
	uint type;
	MemoryPagePool* pool; // type 1

	void init()
	{
		mem = 0;
		memUsage = 0;
		pageSize = 0;
		addressCount = 0;
		forn(MPB_STACK) lastAddr[i] = 0;
		siz = MPB_STACK;
		beg = 0;
		end = 0;
		cnt = 0;
		type = 0;
		pool = 0;
	}
	void push(uint_t address)
	{
		lastAddr[end++] = address;
		if(end == siz)
			end = 0;
		if(cnt < siz)
			cnt++;
		if(cnt == siz)
			beg = end;
	}
	void pop()
	{
		if(!cnt) return;
		end = (beg + cnt - 1) % siz;
		cnt--;
	}
	uint_t last()
	{
		if(!cnt) return 0;
		return lastAddr[(beg + cnt - 1) % siz];
	}
	uint freeSize()
	{
		return pageSize - memUsage;
	}
	bool valid(void* p)
	{
		return (p >= mem and p < mem + pageSize);
	}
	void reset()
	{
		beg = 0;
		end = 0;
		cnt = 0;
		memUsage = 0;
		addressCount = 0;
	}
};

struct MemoryPagePool
{
	MemoryPageBlock* list; // fixed count
	uint firstAvail; // index in list
	uint chunkAvail; // count
	MemoryPageBlock* actual; // current

	void init(uint n, uint chunksz, char* memory)
	{
		firstAvail = 0;
		chunkAvail = n;
		forn(n)
		{
			list[i].init();
			list[i].beg = i+1;
			list[i].type = 2;
			list[i].mem = memory;
			list[i].pageSize = chunksz;
			memory += chunksz;
		}
	}

	MemoryPageBlock* getChunk()
	{
		assert(chunkAvail);
		actual = list + firstAvail;
		firstAvail = actual->beg;
		actual->beg = 0;
		chunkAvail--;
		return actual;
	}

	void freeChunk(MemoryPageBlock* chunk, uint n)
	{
		chunk->reset();
		chunk->beg = firstAvail;
		firstAvail = n;
		chunkAvail++;
		if(actual == chunk)
			actual = 0;
	}

	void reset(uint n)
	{
		firstAvail = 0;
		chunkAvail = n;
		forn(n)
		{
			list[i].beg = i+1;
			list[i].type = 2;
		}
	}
};

MemoryPage* MemoryPageCreate(void* mem)
{
	assert(mem);
	return new(mem) MemoryPage;
}

void* MemoryPageAlloc(MemoryPage* mp, uint size, uint align)
{
	return mp->Alloc(size, align);
}

void* MemoryPage::Alloc(uint size, uint alignArg)
{
	bool overflow = 0;
	uint alignsz = 0;

#if ALLOCATOR_MODE & 0x04
	if(!alignArg)
	{
		alignsz = ALLOCATOR_ALIGN;
		assert((alignsz & alignsz-1) == 0);
	}
#endif
	if(alignArg) // high priority than ALLOCATOR_ALIGN
	{
		alignsz = alignArg;
		assert((alignsz & alignsz-1) == 0);
	}

	if(size + alignsz < chunkSize)
		return AllocChunk(size, alignsz);

	// stdprintf("\nsz %u\n\n", size);

	if(!cmp || cmp->freeSize() < size + alignsz)
	{
		cmp = 0;
		// find free mem page
		forn(list.size())
		{
			if(list[i]->type == 0 and size + alignsz <= list[i]->freeSize())
			{
				cmp = list[i];
				if(cmp->memUsage != 0)
					break;
			}
		}
		if(cmp && cmp->memUsage == 0 && freePages)
			freePages--;
	}

	if(!cmp)
	{
		uint mp_sz = sizeof(MemoryPageBlock) + alignsz;
		uint sm_sz = 0;

		if(size + mp_sz > pageSize)
		{
			overflow = 1;
			sm_sz = size + mp_sz + 16;
		}
		else
			sm_sz = pageSize;

		/*************************************
		 *     Memory placement              *
		 *  1. memory page block             *
		 *  2. memory user                   *
		 *************************************/

		char* s_mem = 0;
		s_mem = (char*) SysAlloc(sm_sz);

		cmp = (MemoryPageBlock*) s_mem;
		cmp->init();
		cmp->mem = s_mem + mp_sz;
		cmp->pageSize = sm_sz - mp_sz;

		list.push_back(cmp);
		allocCount++;
		if(allocCount > allocCountMax)
			allocCountMax = allocCount;
		allocSizeCur += cmp->pageSize;
		if(allocSizeCur > allocSizeMax)
			allocSizeMax = allocSizeCur;

		/* static uint mmx = 0;
		if(size > mmx)
		{
			mmx = size;
			stdprintf("\nmpg %u\n\n", size);
		} */
	}

	cmp->addressCount++;
	char* memp = cmp->mem + cmp->memUsage;
	uint usize = size;
	if(alignsz)
	{
		uint an = alignSize(memp, alignsz);
		usize += an;
		memp += an;
	}
	cmp->memUsage += usize;
	cmp->push((uint_t) memp);
	if(overflow) cmp = 0;
	return memp;
}

void* MemoryPage::AllocChunk(uint size, uint alignsz)
{
	if(!pool or !pool->actual or pool->actual->freeSize() < size + alignsz)
	{
		if(chunkFree == 0)
		{
			// main block type 1, pool, list blocks, .. memory ..
			uint mem_size = sizeof(MemoryPagePool) + sizeof(MemoryPageBlock) * (chunkCount+1) + pageSize + 32;
			char* mem_block = (char*) SysAlloc(mem_size);
			MemoryPageBlock* page_block = (MemoryPageBlock*) mem_block;
			page_block->init();
			page_block->type = 1;
			page_block->pageSize = pageSize;
			mem_block += sizeof(MemoryPageBlock);
			page_block->pool = (MemoryPagePool*) mem_block;
			mem_block += sizeof(MemoryPagePool);
			pool = page_block->pool;
			pool->list = (MemoryPageBlock*) mem_block;
			mem_block += sizeof(MemoryPageBlock) * chunkCount;
			page_block->mem = mem_block;
			pool->init(chunkCount, chunkSize, mem_block);
			pool->getChunk();
			list.push_back(page_block);
			chunkFree = chunkCount - 1;
			allocCount++;
			if(allocCount > allocCountMax)
				allocCountMax = allocCount;
			allocSizeCur += pageSize;
			if(allocSizeCur > allocSizeMax)
				allocSizeMax = allocSizeCur;
		}
		else // have free chunks
		{
			if(!pool or !pool->chunkAvail)
			{
				// search free block in pool
				forn(list.size())
				{
					if(list[i]->type == 1 and list[i]->pool->chunkAvail)
					{
						pool = list[i]->pool;
						break;
					}
				}
			}
			if(pool->chunkAvail == chunkCount)
			{
				assert(freePools);
				freePools--;
			}
			pool->getChunk();
			chunkFree--;
		}
	}

	assert(pool);
	MemoryPageBlock* pb = pool->actual;
	uint usize = size;
	char* memp = pb->mem + pb->memUsage;
	if(alignsz)
	{
		uint an = alignSize(memp, alignsz);
		usize += an;
		memp += an;
	}
	pb->memUsage += usize;
	pb->push((uint_t) memp);
	pb->addressCount++;
	return memp;
}

void MemoryPageFree(MemoryPage* mp, void* p)
{
	mp->Free(p);
}

void MemoryPage::Free(void* p)
{
	if(!mpg or !mpg->valid(p))
	{
		forn(list.size())
		{
			if(list[i]->valid(p))
			{
				mpg = list[i];
				mpgi = i;
				break;
			}
		}
	}

	if(mpg and mpg->type == 1)
		return FreeChunk(p);

	if(!mpg or !mpg->addressCount)
		return;

	if(mpg->addressCount)
		mpg->addressCount--;

	if(mpg->addressCount == 0)
	{
		// delete, overflow
		if(mpg->pageSize > pageSize || freePages > 0)
		{
			allocSizeCur -= mpg->pageSize;
			freeCount++;
			allocCount--;
			list.erase_swap(mpgi);
			SysFree(mpg);
			if(mpg == cmp)
				cmp = 0;
			mpg = 0;
		}
		else // set page state 'empty'
		{
			freePages++;
			mpg->reset();
			if(mpg == cmp)
				cmp = 0;
			mpg = 0;
		}
	}
	// check last address
	elif(mpg->last() == (uint_t) p)
	{
		mpg->memUsage -= (uint_t) mpg->mem + mpg->memUsage - (uint_t) p;
		mpg->pop();
	}
}

void MemoryPage::FreeChunk(void* p)
{
	uint offset = (char*)p - mpg->mem;
	uint index = offset / chunkSize;
	assert(index < chunkCount);
	MemoryPageBlock* chunk = mpg->pool->list + index;

	if(chunk->addressCount)
		chunk->addressCount--;

	// free chunk
	if(chunk->addressCount == 0)
	{
		mpg->pool->freeChunk(chunk, index);
		chunkFree++;

		// pool not empty
		if(mpg->pool->chunkAvail < chunkCount) return;

		// free pool
		if(freePools > 0)
		{
			// stdprintf("\nfree pool \n");
			allocSizeCur -= mpg->pageSize;
			freeCount++;
			allocCount--;
			if(mpg->pool == pool)
				pool = 0;
			list.erase_swap(mpgi);
			SysFree(mpg);
			chunkFree -= chunkCount;
			mpg = 0;
		}
		else // reset pool
		{
			// stdprintf("\nempty pool \n");
			freePools++;
			mpg->pool->reset(chunkCount);
			mpg->reset();
			if(mpg->pool == pool)
				pool = 0;
			mpg = 0;
		}
	}
	// check last address
	elif(chunk->last() == (uint_t) p)
	{
		chunk->memUsage -= (uint_t) chunk->mem + chunk->memUsage - (uint_t) p;
		chunk->pop();
	}
}

bool MemoryPageValid(MemoryPage* mp, void* p, uint size)
{
	// check pointer placed in memory page block and size
	if(mp->cmp and mp->cmp->valid(p))
	{
		return PSIZE(p) == size;
	}
	else
	{
		forn(mp->list.size())
		{
			if(mp->list[i]->valid(p))
			{
				return PSIZE(p) == size;
			}
		}
	}
	return 0;
}

void MemoryPageClear(MemoryPage* mp)
{
	forn(mp->list.size())
	{
		SysFree(mp->list[i]);
		mp->freeCount++;
	}
	mp->list.clear();
}

void MemoryPageInfo(MemoryPage* mp)
{
	char cvs[3][20] = {0};
	_conv_bytes_s(cvs[0], mp->pageSize, 1);
	_conv_bytes_s(cvs[1], mp->chunkSize, 1);
	_conv_bytes_s(cvs[2], mp->allocSizeMax, 1);
	allocatorPrintf("memory page     %s\npool   block    %s\nsystem allocate %u, size %s\nsystem deletion %u\n",
	                cvs[0], cvs[1], mp->allocCountMax, cvs[2], mp->allocCountMax);
}

void MemoryPageSetSize(MemoryPage* mp, uint size, uint chunks)
{
	// min size 4 kb
	assert(size >= 4096);
	mp->pageSize = size;
	mp->chunkSize = chunks;
	mp->chunkCount = size / chunks;
	// stdprintf("chunkCount %u\n", mp->chunkCount);
	// stdprintf("chunkSize %u\n", mp->chunkSize);
}

void MemoryPageGetSizes(MemoryPage* mp, char* size, char m)
{
	_conv_bytes_s(size, !m ? mp->pageSize : mp->allocSizeCur, 2);
}

// align to up
byte alignSize(void* address, byte align)
{
	uint_t ptr = (uint_t) address;
	byte mod = ptr & (align - 1);
	if(!mod) return 0;
	return align - mod;
}