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
#ifndef _TAH_SmartPtr_h_
#define _TAH_SmartPtr_h_

#include <Common/Config.h>

namespace tas
{

/** Smart pointer, template type.
  * @param T Pointer type to single object or array of objects, without '*'.
  * @remark Like unique_ptr.
  */
template<class T>
class SmartPtr
{
public:

	SmartPtr()
	{
		obj = 0;
		array = 0;
	}

	SmartPtr(T* pobj, bool arrayo = 0)
	{
		obj = pobj;
		array = arrayo;
	}

	~SmartPtr()
	{
		release();
	}

	SmartPtr& set(T* pobj, bool arrayo = 0)
	{
		assert(pobj);
		if(obj != pobj) // Avoid self assignment
		{
			release();
			obj = pobj;
			array = arrayo;
		}
		return *this;
	}

	SmartPtr& reset()
	{
		release();
		obj = 0;
		array = 0;
		return *this;
	}

	/*

	// constructor copy
	SmartPtr(const SmartPtr& smp)
	{
		obj = smp.obj;
	}

	SmartPtr& operator = (SmartPtr& smp)
	{
		if(this != &smp) // Avoid self assignment
		{
			// delete old ref counter and obj
			release();
			obj = smp.obj;
			array = smp.array;
		}
		return *this;
	}

	T* operator = (T* pObj)
	{
		assert(pObj);
		if(obj != pObj) // Avoid self assignment
		{
			release();
			obj = pObj;
		}
		return obj;
	}

	*/

	T& operator * ()
	{
		assert(obj && "obj is 0, smp, op *");
		return *obj;
	}

	T& operator [] (uint i)
	{
		assert(obj && "obj is 0, smp, op []");
		return obj[i];
	}

	T* operator -> ()
	{
		assert(obj && "obj is 0, smp, op ->");
		return obj;
	}

	T* ptr()
	{
		return obj;
	}

private:

	T* obj;
	bool array;

	void release()
	{
		if(array)
			safe_delete_array(obj);
		else
			safe_delete(obj);
	}

	// constructor copy
	SmartPtr(const SmartPtr& smp)
	{
		// obj = smp.obj;
		noop;
	}

	SmartPtr& operator = (SmartPtr& smp)
	{
		noop;
		return *this;
	}

	T* operator = (T* pObj)
	{
		noop;
		return obj;
	}

};

}

#endif