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
#include <Common/platform/swindows.h>
#include <Common/UString.h>
#include <Common/StringLib.h>
#include <stdio.h>

#define _printf noop

namespace tas
{

static int compare_impl(const char* d, const char* s, uint n, bool noCase);
uint stringLength(const char* src);
uint wstringLength(const wchar_t* src);

UString::UString()
{
	_printf("UString()\n");
	mString = 0;
	mLength = 0;
	mCapacity = 0;
}

UString::UString(const char* s)
{
	_printf("UString(char*) [%s]\n", s);
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s, stringLength(s));
}

UString::UString(const wchar_t* s)
{
	_printf("UString(wchar_t*) [%ls]\n", s);
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s);
}

UString::UString(const UString& s)
{
	_printf("UString(const UString&) copy ctor\n");
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s.p(), s.mLength);
	_printf("UString(const UString&) copy ctor end\n");
}

UString::~UString()
{
	if(mString != 0 and mLength > 0) _printf("destructor [%s]\n", mString);
	safe_delete_array(mString);
}

uint UString::length() const
{
	return mLength;
}

uint UString::capacity() const
{
	return mCapacity;
}

char* UString::p() const
{
	return mString;
}

void UString::reallocate()
{
	char* temp = new char[mCapacity];
	forn(mLength) temp[i] = mString[i];
	temp[mLength] = '\0';
	safe_delete_array(mString);
	mString = temp;
}

void UString::resize(uint n)
{
	resize(n, 0);
}

void UString::resize(uint n, char c)
{
	if(n + 1 > mCapacity)
	{
		mCapacity = n + 1;
		reallocate();
	}
	if(n > mLength)
	{
		for(uint i = mLength; i < n; i++)
			mString[i] = c;
	}
	mLength = n;
	mString[mLength] = 0;
}

void UString::reserve(uint newCapacity)
{
	_printf("reserve %d %d\n", newCapacity, mCapacity);
	if(newCapacity <= mCapacity) return;
	mCapacity = newCapacity;
	reallocate();
}

void UString::clear()
{
	mLength = 0;
	if(mString)
		mString[0] = '\0';
}

bool UString::empty()
{
	return mLength == 0;
}

uint UString::update()
{
	if(!mString or !mCapacity)
		return mLength;
	char* s = mString;
	uint n = mCapacity;
	while(n-- and *s != '\0' and s++);
	if(*s != '\0')
	{
		mLength = mCapacity - 1;
		mString[mLength] = '\0';
	}
	else
		mLength = s - mString;
	return mLength;
}

UString& UString::assign(const char* s)
{
	return assign(s, stringLength(s));
}

UString& UString::assign(const char* s, uint n)
{
	if(!s)
	{
		clear();
		return *this;
	}
	if(!n)
	{
		reserve(2);
		clear();
		return *this;
	}
	reserve(n + 1);
	_printf("UString::assign(%s) %u\n", mString, n);
	mLength = n;
	char* d = mString;
	while(n--) *d++ = *s++;
	*d = '\0';
	return *this;
}

UString& UString::assign(const wchar_t* s)
{
	return assign(s, wstringLength(s));
}

UString& UString::assign(const wchar_t* s, uint n)
{
	if(!s)
	{
		clear();
		return *this;
	}
	if(!n)
	{
		reserve(2);
		clear();
		return *this;
	}
	uint length = WideCharToMultiByte(CP_UTF8, 0, s, n, 0, 0, 0, 0);
	if(!length)
	{
		reserve(2);
		mString[0] = '\0';
		return *this;
	}
	reserve(length + 1);
	WideCharToMultiByte(CP_UTF8, 0, s, n, mString, length, 0, 0);
	mLength = length;
	mString[mLength] = '\0';
	_printf("UString::assign(wchar) %s %u\n", mString, mLength);
	return *this;
}

UString& UString::assign(const UString& s)
{
	if(this == &s)
		return *this;
	return assign(s.p(), s.length());
}

UString& UString::assign(const UString& s, uint p, uint n)
{
	if(this == &s)
		return *this;
	if(p >= s.length()) return *this;
	uint l = s.length() - p;
	if(n > l) n = l;
	return assign(s.p() + p, n);
}

UString& UString::substrs(uint p, uint n)
{
	if(!mLength or p >= mLength or !n)
		return *this;
	uint l = mLength - p;
	if(n > l) n = l;
	mLength = n;
	char* d = mString;
	char* s = mString + p;
	while(n--) *d++ = *s++;
	*d = 0;
	return *this;
}

UString& UString::swap(UString& s)
{
	// x = a
	char* string_s = mString;
	uint  length_s = mLength;
	uint  capacity_s = mCapacity;
	// a = b
	mString = s.mString;
	mLength = s.mLength;
	mCapacity = s.mCapacity;
	// b = x
	s.mString = string_s;
	s.mLength = length_s;
	s.mCapacity = capacity_s;
	return *this;
}

UString& UString::tolower()
{
	if(!mLength) return *this;
	CharLowerA(mString);
	return *this;
}

UString& UString::toupper()
{
	if(!mLength) return *this;
	CharUpperA(mString);
	return *this;
}

UString& UString::tohex()
{
	if(!mLength) return *this;
	uint len = mLength;
	resize(mLength * 3);
	byte b0 = mString[0];
#define HEX(c) (c < 10 ? c + 48 : c + 55)
	for(uint i = 0, j = len; i < len; i++, j += 2)
	{
		b0 = mString[i];
		if(b0 < 16)
		{
			mString[j] = '0';
			mString[j+1] = HEX(b0);
		}
		else
		{
			mString[j] = HEX((b0 / 16));
			mString[j+1] = HEX((b0 % 16));
		}
	}
#undef HEX
	substrs(len);
	return *this;
}

UString& UString::format(const char* fmt, ...)
{
	if(!fmt or *fmt == '\0') return *this;
	va_list args;
	va_start(args, fmt);
#ifdef TAA_VSNPRINTF
	uint size = vsnprintf(0, 0, fmt, args);
	if(!size) return *this;
	resize(size);
	size = vsnprintf(mString, size+1, fmt, args);
	mString[mLength] = '\0';
#else
	resize(512);
	vsprintf(mString, fmt, args);
	update();
#endif
	va_end(args);
	return *this;
}

char& UString::at(uint i)
{
	assert(i < mCapacity);
	return mString[i];
}

char& UString::operator [] (uint i)
{
	assert(i < mCapacity);
	return mString[i];
}

UString& UString::operator = (const char* s)
{
	return assign(s);
}

UString& UString::operator = (const wchar_t* s)
{
	return assign(s);
}

UString& UString::operator = (const UString& s)
{
	return assign(s);
}

UString& UString::move(UString& s)
{
	safe_delete_array(mString);
	mString = s.mString;
	mLength = s.mLength;
	mCapacity = s.mCapacity;
	s.mString = 0;
	s.mLength = 0;
	s.mCapacity = 0;
	return *this;
}

}