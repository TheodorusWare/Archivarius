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
#ifndef _TAH_Array_h_
#define _TAH_Array_h_

/** @defgroup containers Containers
    @{
*/

#include <Common/Config.h>
#include <Common/Container/Allocator.h>
#include <typeinfo>

namespace tas
{

/** Templated dynamic array.
  * @param T Generic array elements type.
  * @param A Allocator type.
  */
template<class T, class A = Allocator<T> >
class Array
{
protected:

	A  allocator;    ///< Allocator.
	T* arrayGen;     ///< Generic array.
	uint mSize;      ///< Current size.
	uint mCapacity;  ///< Array capacity.

	void initdef()
	{
		arrayGen = 0;
		mCapacity = 0;
		mSize = 0;
	}

	/** reallocate method */
	virtual Array& assign(T& a, T& b)
	{
		// _printf("array assign\n");
		a = b;
		return *this;
	}

public:

	/** Constructors. */
	Array()
	{
		initdef();
	}

	Array(uint size)
	{
		initdef();
		resize(size);
	}

	/** Constructor copy.
	  * @param obj Object reference for initialization.
	  */
	Array(const Array& obj)
	{
		initdef();
		uint n = obj.mSize;
		if(n == 0) return;
		mCapacity = mSize = n;
		arrayGen = allocator.allocate(n);
		forn(n) arrayGen[i] = obj.arrayGen[i];
	}

	/** Destructor. */
	virtual ~Array()
	{
		clear();
	}

	/** Push new element at the end.
	  * @param elm Pushed element to array.
	  */
	void push_back(const T& elm)
	{
		if(mSize == mCapacity)
		{
			// first style, assign mCapacity * 2
			mCapacity = mCapacity == 0 ? 2 : mCapacity * 2;

			// std::vector style, inc mCapacity / 2
			// note: need many reallocate memory
			// mCapacity += mSize == 0 ? 2 : mSize / 2;

			reallocate();
		}
		// arrayGen[mSize++] = const_cast<T&>(elm);
		arrayGen[mSize++] = elm;
		// assign(arrayGen[mSize++], elm);
	}

	/** Pop last element. */
	void pop_back()
	{
		assert(mSize--);
	}

	T* begin()
	{
		return arrayGen;
	}

	T* end()
	{
		return arrayGen + mSize;
	}

	T& first()
	{
		assert(mSize);
		return arrayGen[0];
	}

	T& last()
	{
		assert(mSize);
		return arrayGen[mSize - 1];
	}

	/** Insert element at index n.
	  * @param elm Inserted element.
	  * @param n Position inserted element.
	  */
	void insert(const T& elm, uint n)
	{
		assert(n < mSize);

		if(n >= mSize)
		{
			push_back(elm);
			return;
		}

		if(mSize == mCapacity)
		{
			mCapacity = mCapacity == 0 ? 2 : mCapacity * 2;
			reallocate();
		}

		forn(mSize - n)
		arrayGen[mSize - i] = arrayGen[mSize - i - 1];
		arrayGen[n] = const_cast<T&>(elm);
		mSize++;
	}

	/** Reserve memory.
	  * @param newCapacity New array capacity.
	  * @remark If newCapacity great current array capacity.
	  */
	void reserve(uint newCapacity)
	{
		if(newCapacity <= mCapacity) return;
		mCapacity = newCapacity;
		reallocate();
	}

	/** Reallocate memory.
	  */
	void reallocate()
	{
		// _printf("reallocate beg %u %u\n", mCapacity, mSize);
		T* temp = allocator.allocate(mCapacity);
		// forn(mSize) temp[i] = arrayGen[i];
		forn(mSize) assign(temp[i], arrayGen[i]);
		allocator.deallocate(arrayGen);
		arrayGen = temp;
		// _printf("reallocate end %u\n", mCapacity);
	}

	/** Clear array memory.
	  */
	void clear()
	{
		if (arrayGen == 0 || mCapacity == 0) return;
		allocator.deallocate(arrayGen);
		arrayGen = 0;
		mCapacity = 0;
		mSize = 0;
	}

	/** Not clear array memory, only null variables.
	  * @remarks Please not use this methon, he stupid.
	  */
	void clears()
	{
		mCapacity = 0;
		mSize = 0;
	}

	/** Clear pointers in array.
	  */
	void clear_ptr()
	{
		if(arrayGen == 0 || mCapacity == 0) return;
		forn(mSize)
		safe_delete(arrayGen[i]);
		clear();
	}

	/** Erase elements from array use shift.
	  * @param ix Index of the element to erase.
	  * @param nc Count of the elements to erase.
	  */
	void erase(uint ix, uint nc = 1)
	{
		assert(ix < mSize);

		if(nc >= mSize)
		{
			mSize = 0;
			return;
		}

		if(ix >= mSize - nc)
		{
			mSize = ix;
			return;
		}

		forn(mSize - ix - nc)
		arrayGen[ix + i] = arrayGen[ix + i + nc];
		mSize -= nc;
	}

	/** Erase one element from array use swap with last element.
	  * @param i Number of the element to erase.
	  */
	void erase_swap(uint i)
	{
		assert(i < mSize);
		if(i < mSize - 1)
			arrayGen[i] = arrayGen[mSize - 1];
		mSize--;
	}

	/** Erase one element from array.
	  * @param n  Numbers of the element to erase.
	  * @param nc Count of the elements to erase.
	  */
	void erase_multi(uint* n, uint nc)
	{
		uint x = 0, u = 0;

		T* temp = allocator.allocate(mCapacity);

		forn(mSize)
		{
			if(x >= nc || i != n[x]) // save
			{
				temp[u++] = arrayGen[i];
			}
			else // deleted
				x++;
		}

		allocator.deallocate(arrayGen);
		arrayGen = temp;
		mSize -= nc;
	}

	void erase_elm(const T& val)
	{
		int fv = find(val);
		if(fv != -1) erase(fv);
	}

	/** Resize array.
	  * @param n New array size.
	  */
	void resize(uint n)
	{
		if(n > mCapacity)
		{
			mCapacity = n;
			reallocate();
		}
		mSize = n;
	}

	/** Is empty array. */
	bool empty()
	{
		return mSize == 0;
	}

	/** Get array size.
	  * @return Current array size.
	  */
	uint size()
	{
		return mSize;
	}

	/** Get array capacity.
	  * @return Current memory size of the array.
	  */
	uint capacity()
	{
		return mCapacity;
	}

	/** Array is full. */
	bool full()
	{
		return mSize == mCapacity;
	}

	/** Find value in array.
	  * @param val Search value.
	  * @return The position of the founded value, else -1.
	  */
	int find(T val)
	{
		forn(mSize)
		when(arrayGen[i] == val)
		return i;
		return -1;
	}

	/** Multi find values in array.
	  * @param[in] val Search value.
	  * @param[out] nvals Count indexes.
	  * @return Pointers to indexes, else 0.
	  */
	int* multi_find(T val, uint& nvals)
	{
		uint nv = 0;
		nvals = 0;
		int* vals = allocator.allocate(mSize);

		forn(mSize)
		when (arrayGen[i] == val)
		vals[nv++] = i;

		if(nv)
		{
			int* fvals = allocator.allocate(nv);
			forn(nv)
			fvals[i] = vals[i];
			allocator.deallocate(vals);
			nvals = nv;
			return fvals;
		}
		else
		{
			allocator.deallocate(vals);
			return 0;
		}
		return 0;
	}

	/** Get element by index.
	  * @param i Indexed value.
	  * @return pointers to indexes, else error.
	  */
	T& operator[] (uint i)
	{
		if(i >= mSize)
			_printf("array T = %s\n", typeid(T).name());
		assert(i < mSize);
		return arrayGen[i];
	}

	/** Assign array.
	  * @param obj Input array.
	  * @return reference to current array.
	  */
	Array& operator= (const Array& obj)
	{
		if(obj.mSize == 0 or this == &obj)
			return *this;
		if(obj.mSize > mCapacity)
		{
			mCapacity = obj.mSize;
			allocator.deallocate(arrayGen);
			arrayGen = allocator.allocate(mCapacity);
		}
		forn(obj.mSize)
		arrayGen[i] = obj.arrayGen[i];
		mSize = obj.mSize;
		return *this;
	}

	/** Swap elements of array.
	  * @param o1 First element index.
	  * @param o2 Second element index.
	  */
	void swap(uint o1, uint o2)
	{
		T temp = arrayGen[o1];
		arrayGen[o1] = arrayGen[o2];
		arrayGen[o2] = temp;
	}

	/** Selection sort elements of array.
	  * @param compare Template function for comparison two elements.
	  * @remarks Compare function must return bool value
	  * after comparison two elements. <br>
	  * If function comparison (o1 > o2), increase. <br>
	  * If function comparison (o1 < o2), decrease.
	  */
	template <class Compare>
	void sort(Compare compare)
	{
		uint res = 0;
		for(uint i = 0; i < mSize; i++)
		{
			res = i;
			for(uint u = i+1; u < mSize; u++)
			{
				if(compare(arrayGen[res], arrayGen[u]))
					res = u;
			}
			if(i != res)
				swap(i, res);
		}
	}

	/** Quick sort elements of array.
	  * @param compare Template function for comparison two elements.
	  * @remarks Compare function must return int value,
	  * relation between two elements, difference A and B.
	  * Increase sorting, difference A and B.
	  * Decrease sorting, difference B and A.
	  */
	template <class Compare>
	void qsort(Compare compare)
	{
		if(mSize < 2) return;
		qsortRange(0, mSize, compare);
	}

	template <class Compare>
	void qsortRange(int start, int end, Compare compare)
	{
		if(start + 1 >= end) return;
		T pivot = arrayGen[start + ((end - start) / 2)];
		int i, j;
		for(i = start, j = end - 1; ; i++, j--)
		{
			while(compare(arrayGen[i], pivot) < 0) i++;
			while(compare(arrayGen[j], pivot) > 0) j--;
			if(i >= j) break;
			swap(i, j);
		}
		qsortRange(start, i, compare);
		qsortRange(i, end, compare);
	}

	/** Selection sort strings elements of array.
	  * @param increase by 1, else decrease.
	  * @param noCase Without case sensivity by 1, else with.
	  */
	void sortStringMode(byte increase)
	{
#define A arrayGen[res]
#define B arrayGen[u]
#define CMP A.compare(B, 0, -1)
		uint res = 0;
		for(uint i = 0; i < mSize; i++)
		{
			res = i;
			for(uint u = i+1; u < mSize; u++)
			{
				if(increase ? CMP > 0 : CMP < 0)
					res = u;
			}
			if(i != res)
				arrayGen[i].swap(A);
		}
#undef A
#undef B
#undef CMP
	}

	/** Binary insert element to array.
	  * @param val Inserted element.
	  * @param compare Template function for comparison two elements.
	  * @return index element.
	  * @remarks Compare function must return int value
	  * after comparison two elements, difference A and B.
	  */
	template <class Compare>
	int insert_binary(const T& val, Compare compare)
	{
		if(mSize == 0)
		{
			push_back(val);
			return 0;
		}

		int mid = 0;
		int left = 0;
		int right = mSize - 1;

		if(compare(val, arrayGen[0]) < 0)
		{
			insert(val, 0);
			return 0;
		}

		if(compare(val, arrayGen[right]) > 0)
		{
			push_back(val);
			return mSize - 1;
		}

		while(left < right)
		{
			mid = (left + right) / 2;

			if(compare(val, arrayGen[mid]) > 0 and compare(val, arrayGen[mid + 1]) < 0)
			{
				insert(val, mid+1);
				return mid+1;
			}

			if(compare(val, arrayGen[mid]) == 0)
				return mid;
			else if(compare(val, arrayGen[mid]) < 0)
				right = mid - 1;
			else if(compare(val, arrayGen[mid]) > 0)
				left = mid + 1;
		}
		return -1;
	}

	/** Binary search element in array.
	  * @param val Searched element.
	  * @param compare Template function for comparison two elements.
	  * See insert_binary.
	  * @return index found element, else -1.
	  */
	template <class Compare>
	int search_binary(const T& val, Compare compare)
	{
		if(mSize == 0) return -1;
		int mid = 0;
		int left = 0;
		int right = mSize - 1;

		if(compare(val, arrayGen[0]) < 0)
			return -1;
		if(compare(val, arrayGen[right]) > 0)
			return -1;

		while(left <= right)
		{
			mid = (left + right) / 2;
			if(compare(val, arrayGen[mid]) == 0)
				return mid;
			else if(compare(val, arrayGen[mid]) < 0)
				right = mid - 1;
			else if(compare(val, arrayGen[mid]) > 0)
				left = mid + 1;
		}
		return -1;
	}
};

/** Templated dynamic array with move semantic.
  */
template<class T, class A = Allocator<T> >
class ArrayMove: public Array<T, A>
{
	ArrayMove& assign(T& a, T& b)
	{
		// _printf("ArrayMove assign\n");
		a.move(b);
		return *this;
	}

public:

	/** Push new element at the end by moving.
	  * @param elm Pushed element to array.
	  */
	void move_back(const T& elm)
	{
		if(this->mSize == this->mCapacity)
		{
			this->mCapacity = this->mCapacity == 0 ? 2 : this->mCapacity * 2;
			this->reallocate();
		}
		assign(this->arrayGen[this->mSize++], const_cast<T&>(elm));
	}
};

} // namespace

/** example how to declare template function
template<class T>
int Array<T>::find(T val)
{
		to do ...
}
*/

/** @} */

#endif
