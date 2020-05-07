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
#ifndef _TAH_UString_h_
#define _TAH_UString_h_

#include <Common/Config.h>

namespace tas
{

/// UTF-8
class TAA_LIB UString
{
public:

	UString();
	UString(const char* s);
	UString(const wchar_t* s);
	UString(const UString& s);
	~UString();

	char* p() const;
	uint length() const;
	uint capacity() const;

	void reallocate();
	void resize(uint n); /// string
	void resize(uint n, char c); /// string
	void reserve(uint newCapacity); /// memory
	void clear();  /// set empty
	bool empty();  /// is empty ?
	uint update(); /// length

	/** Assign buffer.
	  * @param s Insertion string or buffer.
	  * @param n UString or buffer length.
	  */
	UString& assign(const UString& s);
	UString& assign(const UString& s, uint p, uint n);
	UString& assign(const char*   s);
	UString& assign(const char*   s, uint n);

	/** Assign UTF-16 string. */
	UString& assign(const wchar_t* s);
	UString& assign(const wchar_t* s, uint n);

	/** Generate substring inside current. */
	UString& substrs(uint p, uint n = -1);

	/** Swap two string. */
	UString& swap(UString& s);

	UString& tolower();
	UString& toupper();

	/** Convert each byte string in two byte hex representation. */
	UString& tohex();

	UString& format(const char* fmt, ...);

	char&   at          (uint i);
	char&   operator [] (uint i);

	UString& operator = (const char*   s);
	UString& operator = (const wchar_t* s);
	UString& operator = (const UString& s);

	/** Move s in current object. */
	UString& move(UString& s);

private:

	char* mString; /// with terminating null-character ('\0') at the end.
	uint  mLength;
	uint  mCapacity;
};

}

#endif