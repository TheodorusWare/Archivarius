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
#ifndef _TAH_HashTable_h_
#define _TAH_HashTable_h_

#include <Common/Config.h>
#include <Common/Bitwise.h>

namespace tas
{

/** Hash table fixed size without reallocate memory.
  *
  * Type <T> must have next procedures:
  *
  * void reset(uint checksum)
  * Description: reset object to null state, and save checksum.
  *
  * uint priority()
  * Description: return priority object.
  *
  * uint checksum()
  * Description: return checksum (hash) of object.
  *
  * Search method
  * Table method get() check first 3 elements.
  * In a situation where all three elements busy,
  * the element with the minimum priority is removed.
  *
  * Table method find() check first 3 elements.
  * If item founded, return him with index,
  * else return null item with index -1.
  */
template<class T>
class HashTable
{
	T* table;
	uint sizen;
public:
	uint index; // last founded item

	HashTable()
	{
		table = 0;
		sizen = 0;
		index = 0;
	}

	~HashTable()
	{
		safe_delete_ao(table);
	}

	void initialise(uint n)
	{
		table = new T[n];
		sizen = n;
	}

	uint size()
	{
		return sizen;
	}

	template<class TC>
	T& get(uint hash, TC checksum)
	{
		uint start = hash % sizen;
		uint msb = hash >> 8; // hash msb 3 byte
		uint lp = -1; // low priority of founded item
		uint li = hash; // founded item index with low priority
		uint cp = 0; // cur priority
		uint ci = 0; // cur search index
		forn(3)
		{
			index = ci = (start + i * msb) % sizen;
			T& item = table[ci];
			cp = item.priority();
			if(cp == 0)
			{
				item.reset(checksum);
				return item;
			}
			if(checksum == item.checksum())
				return item;
			if(cp < lp)
			{
				lp = cp;
				li = ci;
			}
		}
		T& item = table[li];
		item.reset(checksum);
		index = li;
		return item;
	}

	template<class TC>
	T& find(uint hash, TC checksum)
	{
		uint start = hash % sizen;
		uint msb = hash >> 8; // hash msb 3 byte
		uint ci = 0; // cur search index
		index = -1;
		forn(3)
		{
			ci = (start + i * msb) % sizen;
			T& item = table[ci];
			if(item.priority() == 0)
				return table[0];
			if(checksum == item.checksum())
			{
				index = ci;
				return item;
			}
		}
		return table[0];
	}

	T& operator [] (uint i)
	{
		assert(i < sizen);
		return table[i];
	}
};

}

#endif