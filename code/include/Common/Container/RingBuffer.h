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
#ifndef _TAH_RingBuffer_h_
#define _TAH_RingBuffer_h_

#include <Common/Config.h>
#include <Common/StringLib.h>

namespace tas
{

template<class T>
class RingBuffer
{
public:
	T*   buf; // buffer
	uint bsz; // buffer size
	uint beg; // begin position
	uint end; // end position
	uint cnt; // current count items

	RingBuffer()
	{
		buf = 0;
		bsz = 0;
		beg = 0;
		end = 0;
		cnt = 0;
	}

	~RingBuffer()
	{
		safe_delete_array(buf);
	}

	RingBuffer& operator = (const RingBuffer& rb)
	{
		if(!rb.cnt)
		{
			reset();
			return *this;
		}
		if(rb.cnt > bsz)
		{
			bsz = rb.cnt;
			safe_delete_array(buf);
			buf = new T[bsz];
			menset(buf, 0, bsz * sizeof(T));
		}
		RingBuffer& rbm = const_cast<RingBuffer&>(rb);
		reset();
		forn(rbm.cnt)
		push(rbm.get(i));
		return *this;
	}

	RingBuffer& move(RingBuffer& rb)
	{
		safe_delete_array(buf);
		buf = rb.buf;
		bsz = rb.bsz;
		beg = rb.beg;
		end = rb.end;
		cnt = rb.cnt;
		rb.buf = 0;
		rb.bsz = 0;
		rb.reset();
		return *this;
	}

	TAA_INLINE void clear()
	{
		safe_delete_array(buf);
		reset();
		bsz = 0;
	}

	TAA_INLINE void initialise(uint size)
	{
		safe_delete_array(buf);
		bsz = size;
		buf = new T[bsz];
		menset(buf, 0, bsz * sizeof(T));
	}

	TAA_INLINE void resize(uint size)
	{
		T* inbuf = new T[size];
		mencpy(inbuf, buf, bsz * sizeof(T));
		safe_delete_array(buf);
		buf = inbuf;
		bsz = size;
	}

	TAA_INLINE void reset()
	{
		beg = 0;
		end = 0;
		cnt = 0;
	}

	TAA_INLINE void endup()
	{
		if(beg < cnt >> 1)
			end = cnt;
		else
			beg = 0;
		cnt = end - beg;
	}

	/// pop front, c count
	TAA_INLINE bool pop(uint c)
	{
		if(!cnt)
			return 0;
		if(c >= cnt)
		{
			cnt = 0;
			beg = 0;
			end = 0;
		}
		else
		{
			beg = (beg + c) % bsz;
			cnt -= c;
		}
		return 1;
	}

	/// pop back, c count
	TAA_INLINE bool popb(uint c)
	{
		if(!cnt)
			return 0;
		if(c >= cnt)
		{
			cnt = 0;
			beg = 0;
			end = 0;
		}
		else
		{
			end = (beg + cnt - c) % bsz;
			cnt -= c;
		}
		return 1;
	}

	TAA_INLINE void push(T b)
	{
		// assert(buf and bsz and "ring buffer empty");
		buf[end++] = b;
		if(end == bsz)
			end = 0;
		if(cnt < bsz)
			cnt++;
		else
			beg = end;
	}

	/// p position from begin
	TAA_INLINE T& get(uint p)
	{
		p += beg;
		if(p < bsz) return buf[p];
		assert(p - bsz < bsz);
		return buf[p - bsz];
	}

	/// p position from end
	/// start index of last item is 0
	TAA_INLINE T& last(uint p)
	{
		p++;
		if(p > end)
			return buf[bsz + end - p];
		return buf[end - p];
	}

	/// get item from buffer position with update
	TAA_INLINE T& getp(uint& p)
	{
		if(p < bsz)
			return buf[p++];
		p -= bsz;
		return buf[p++];
	}

	/// calculate buffer position from last item
	TAA_INLINE uint lastp(uint p)
	{
		p++;
		if(p > end)
			p = bsz + end - p;
		else
			p = end - p;
		return p;
	}

	TAA_INLINE bool full()
	{
		return cnt == bsz;
	}
};

}

#endif