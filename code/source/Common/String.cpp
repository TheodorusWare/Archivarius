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
#include <Common/String.h>
#include <stdio.h>

#define _printf noop

namespace tas
{

static int compare_impl(const wchar_t* d, const wchar_t* s, uint n);
uint stringLength(const char* src);
uint wstringLength(const wchar_t* src);

String::String()
{
	_printf("String()\n");
	mString = 0;
	mLength = 0;
	mCapacity = 0;
}

String::String(const wchar_t* s)
{
	_printf("String(wchar_t*) [%s]\n", s);
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s, wstringLength(s));
}

String::String(const char* s)
{
	_printf("String(char*) [%ls]\n", s);
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s);
}

String::String(const String& s)
{
	_printf("String(const String&) copy ctor\n");
	mString = 0;
	mLength = 0;
	mCapacity = 0;
	assign(s.p(), s.mLength);
	_printf("String(const String&) copy ctor end\n");
}

String::~String()
{
	if(mString != 0 and mLength > 0) _printf("destructor [%s]\n", mString);
	safe_delete_array(mString);
}

uint String::length() const
{
	return mLength;
}

uint String::capacity() const
{
	return mCapacity;
}

wchar_t* String::p() const
{
	return mString;
}

void String::reallocate()
{
	_printf("reallocate beg %u %u\n", mCapacity, mLength);
	wchar_t* temp = new wchar_t[mCapacity];
	forn(mLength) temp[i] = mString[i];
	temp[mLength] = 0;
	safe_delete_array(mString);
	mString = temp;
	_printf("reallocate end %u\n", mCapacity);
}

void String::resize(uint n)
{
	resize(n, 0);
}

void String::resize(uint n, wchar_t c)
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

void String::reserve(uint newCapacity)
{
	_printf("reserve %d %d\n", newCapacity, mCapacity);
	if(newCapacity <= mCapacity) return;
	mCapacity = newCapacity;
	reallocate();
}

void String::clear()
{
	mLength = 0;
	if(mString)
		mString[0] = 0;
}

bool String::empty()
{
	return mLength == 0;
}

uint String::update()
{
	if(!mString or !mCapacity)
		return mLength;
	wchar_t* s = mString;
	uint n = mCapacity;
	while(n-- and *s != 0 and s++);
	if(*s != 0)
	{
		mLength = mCapacity - 1;
		mString[mLength] = 0;
	}
	else
		mLength = s - mString;
	return mLength;
}

String& String::assign(const wchar_t* s)
{
	return assign(s, wstringLength(s));
}

String& String::assign(const wchar_t* s, uint n)
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
	_printf("String::assign(%ls) %u\n", mString, n);
	mLength = n;
	wchar_t* d = mString;
	while(n--) *d++ = *s++;
	*d = 0;
	return *this;
}

String& String::assign(const char* s)
{
	return assign(s, stringLength(s));
}

String& String::assign(const char* s, uint n)
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
	uint length = MultiByteToWideChar(CP_UTF8, 0, s, n, 0, 0);
	if(!length)
	{
		reserve(2);
		mString[0] = 0;
		return *this;
	}
	reserve(length + 1);
	MultiByteToWideChar(CP_UTF8, 0, s, n, mString, length);
	mLength = length;
	mString[mLength] = 0;
	_printf("String::assign(wchar) %s %u\n", mString, mLength);
	return *this;
}

String& String::assign(wchar_t c)
{
	return assign(&c, 1);
}

String& String::assign(const String& s)
{
	if(this == &s)
		return *this;
	return assign(s.p(), s.length());
}

String& String::assign(const String& s, uint p, uint n)
{
	if(this == &s)
		return *this;
	if(p >= s.length()) return *this;
	uint l = s.length() - p;
	if(n > l) n = l;
	return assign(s.p() + p, n);
}

String& String::append(const wchar_t* s)
{
	return append(s, wstringLength(s));
}

String& String::append(const wchar_t* s, uint n)
{
	if(!s or !n or *s == 0)
		return *this;
	reserve(mLength + n + 1);
	wchar_t* d = mString + mLength;
	uint j = n;
	while(j--) *d++ = *s++;
	mLength += n;
	mString[mLength] = 0;
	return *this;
}

String& String::append(wchar_t c)
{
	return append(&c, 1);
}

String& String::append(const String& s)
{
	if(this == &s or !s.length())
		return *this;
	return append(s.p(), s.length());
}

String& String::append(const String& s, uint p, uint n)
{
	if(this == &s or p >= s.length())
		return *this;
	uint l = s.length() - p;
	if(n > l) n = l;
	return append(s.p() + p, n);
}

String& String::insert(uint p, const wchar_t* s, uint n)
{
	if(!s or !n or p >= mLength)
		return *this;
	if(!mLength) return assign(s, n);
	if(p == mLength) return append(s, n);
	reserve(mLength + n + 1);
	// shift
	wchar_t* d = mString + mLength + n - 1; // new pos
	wchar_t* e = mString + mLength - 1; // old
	uint j = mLength - p;
	while(j--) *d-- = *e--;
	// copy
	d = mString + p;
	j = n;
	while(j--) *d++ = *s++;
	mLength += n;
	mString[mLength] = 0;
	return *this;
}

String& String::insert(uint p, wchar_t c)
{
	return insert(p, &c, 1);
}

String& String::insert(uint pos, const String& s)
{
	if(this == &s or pos >= mLength or s.length() == 0)
		return *this;
	return insert(pos, s.p(), s.length());
}

String& String::insert(uint pos, const String& s, uint p, uint n)
{
	if(this == &s or pos >= mLength or p >= s.length())
		return *this;
	uint l = s.length() - p;
	if(n > l) n = l;
	return insert(pos, s.p() + p, n);
}

String& String::replace(uint p, uint nr, const wchar_t* s, uint ns)
{
	if(!s or !ns or !nr or !mLength or p >= mLength) return *this;
	uint l = mLength - p; // avail
	if(nr > l) nr = l;
	if(!p and nr == mLength)
		return assign(s, ns);
	if(ns != nr and p + nr < mLength)
	{
		if(ns > nr) // shift forward
		{
			resize(mLength + ns);
			wchar_t* d = mString + mLength + ns - nr - 1;
			wchar_t* ds = mString + mLength - 1;
			uint j = mLength - p - nr;
			while(j--) *d-- = *ds--;
		}
		else // ns < nr, shift backward
		{
			wchar_t* d = mString + p + ns;
			wchar_t* ds = mString + p + nr;
			uint j = mLength - p - nr;
			while(j--) *d++ = *ds++;
		}
	}
	wchar_t* d = mString + p;
	uint j = ns;
	while(j--) *d++ = *s++;
	mLength += ns - nr;
	mString[mLength] = 0;
	return *this;
}

String& String::replace(uint p, uint nr, const String& s)
{
	if(this == &s or p >= mLength or !s.length())
		return *this;
	uint l = mLength - p;
	if(nr > l) nr = l;
	return replace(p, nr, s.p(), s.length());
}

String& String::replace(uint p, wchar_t c)
{
	if(p >= mLength)
		return *this;
	if(c == 0)
	{
		mString[p] = 0;
		mLength = p;
	}
	else
		mString[p] = c;
	return *this;
}

String& String::erase(uint p, uint n)
{
	if(!mString or !mLength or p >= mLength) return *this;
	uint m = mLength - p; // max
	if(n > m) n = m;
	if(p + n == mLength)
	{
		mString[p] = 0;
		mLength = p;
		return *this;
	}
	uint j = mLength - p - n;
	wchar_t* d = mString + p;
	wchar_t* s = mString + p + n;
	while(j--) *d++ = *s++;
	mLength -= n;
	mString[mLength] = 0;
	return *this;
}

String String::substr(uint p, uint n) const
{
	String sub;
	if(!mLength or p >= mLength or !n)
		return sub;
	uint l = mLength - p; // max
	if(n > l) n = l;
	sub.resize(n);
	wchar_t* d = (wchar_t*)sub.p();
	wchar_t* s = mString + p;
	while(n--) *d++ = *s++;
	*d = 0;
	return sub;
}

String& String::substrs(uint p, uint n)
{
	if(!mLength or p >= mLength or !n)
		return *this;
	uint l = mLength - p; // max
	if(n > l) n = l;
	mLength = n;
	wchar_t* d = mString;
	wchar_t* s = mString + p;
	while(n--) *d++ = *s++;
	*d = 0;
	return *this;
}

String String::substrx(wchar_t sp, wchar_t rv, wchar_t m) const
{
	String sub;
	if(!mLength) return sub;
	uint p = rv ? findr(sp) : find(sp);
	if(p != MAX_UINT32)
	{
		if(m)
			sub.assign(*this, p + 1, -1);
		else
			sub.assign(*this, 0, p);
	}
	else
		sub = *this;
	return sub;
}

String& String::substrxs(wchar_t sp, wchar_t rv, wchar_t m)
{
	if(!mLength) return *this;
	uint p = rv ? findr(sp) : find(sp);
	if(p != MAX_UINT32)
	{
		if(m)
			assign(mString, p + 1, mLength - p - 1);
		else
			resize(p);
	}
	return *this;
}

String& String::trim(wchar_t sp, wchar_t rv)
{
	if(!mLength) return *this;
	uint p = rv ? findr(sp) : find(sp);
	if(p != MAX_UINT32)
		resize(p);
	return *this;
}

String& String::trim()
{
	if(!mLength) return *this;
	uint b = findn(' ');
	if(b == -1)
	{
		clear();
		return *this;
	}
	uint e = findnr(' ');
	uint n = e - b + 1;
	substrs(b, n);
	return *this;
}

String& String::swap(String& s)
{
	// x = a
	wchar_t* string_s = mString;
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

uint String::find(const wchar_t* s, uint p) const
{
	return find(s, p, wstringLength(s));
}

uint String::find(const wchar_t* s, uint p, uint n) const
{
	/* find in buffer, may not c-string. */
	assert(p != MAX_UINT32);
	if(!s or !n or !mLength or p + n > mLength) return -1;
	uint l = 0; // avail
	uint j = mLength - p;
	wchar_t* d = mString + p;
	wchar_t* e = mString + mLength;
	for(; j--; d++)
	{
		if(*d == *s)
		{
			if(n == 1) return d - mString;
			wchar_t* dd = d;
			wchar_t* ss = (wchar_t*)s;
			l = e - d;
			if(l < n) return -1;
			l = n;
			for(; l and *dd == *ss; l--, dd++, ss++);
			if(l == 0) return d - mString;
		}
	}
	return -1;
}

uint String::find(wchar_t c, uint p) const
{
	return find(&c, p, 1);
}

uint String::find(const String& s, uint p) const
{
	return find(s.p(), p, s.length());
}

uint String::findr(const wchar_t* s, uint p) const
{
	return findr(s, p, wstringLength(s));
}

uint String::findr(const wchar_t* s, uint p, uint n) const
{
	/* find in buffer, may not c-string. */
	if(!s or !n or !mLength) return -1;
	if(p >= mLength) p = mLength - 1;
	uint l = p + 1; // avail
	if(n > l) return -1;
	uint j = l;
	wchar_t* d = mString + p;
	wchar_t* sr = (wchar_t*)s + n - 1;
	for(; j--; d--)
	{
		if(*d == *sr)
		{
			if(n == 1) return d - mString;
			wchar_t* dd = d;
			wchar_t* ss = (wchar_t*)sr;
			l = d - mString + 1;
			_printu(l);
			if(l < n)
				return -1;
			l = n;
			for(; l and *dd == *ss; l--, dd--, ss--);
			if(l == 0) return dd - mString + 1;
		}
	}
	return -1;
}

uint String::findr(wchar_t c, uint p) const
{
	return findr(&c, p, 1);
}

uint String::findr(const String& s, uint p) const
{
	return findr(s.p(), p, s.length());
}

uint String::findn(wchar_t c, uint p) const
{
	if(!mLength or p >= mLength) return -1;
	wchar_t* s = mString + p;
	for(; *s and *s == c; s++);
	if(*s) return s - mString;
	return -1;
}

uint String::findnr(wchar_t c, uint p) const
{
	if(!mLength) return -1;
	if(p >= mLength) p = mLength - 1;
	uint n = p + 1;
	wchar_t* s = mString + p;
	for(; n and *s == c; n--, s--);
	if(n) return s - mString;
	return -1;
}

int String::compare(const String& s)
{
	return compare_impl(mString, s.p(), min(mLength, s.length()) + 1);
}

int String::compare(const String& s, uint p, uint n)
{
	if(p >= s.length()) return 0;
	uint l = s.length() - p;
	if(n > l) n = l;
	return compare_impl(mString, s.p() + p, n);
}

int String::compare(const wchar_t* s)
{
	uint n = wstringLength(s);
	return compare_impl(mString, s, min(mLength, n) + 1);
}

int String::compare(const wchar_t* s, uint n)
{
	return compare_impl(mString, s, n);
}

String& String::tolower()
{
	if(!mLength) return *this;
	CharLowerW(mString);
	return *this;
}

String& String::toupper()
{
	if(!mLength) return *this;
	CharUpperW(mString);
	return *this;
}

String& String::tohex()
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

String& String::format(const char* fmty, ...)
{
	if(!fmty or *fmty == 0) return *this;
	static wchar_t fmtw[1024];
	const wchar_t* fmt = fmtw;
	uint len = stringLength(fmty);
	forn(len) fmtw[i] = fmty[i];
	fmtw[len] = 0;
	va_list args;
	va_start(args, fmt);
	uint size = vswprintf(0, 0, fmt, args);
	if(!size) return *this;
	resize(size);
	size = vswprintf(mString, size+1, fmt, args);
	va_end(args);
	return *this;
}

String& String::format(const wchar_t* fmt, ...)
{
	if(!fmt or *fmt == 0) return *this;
	va_list args;
	va_start(args, fmt);
	uint size = vswprintf(0, 0, fmt, args);
	if(!size) return *this;
	resize(size);
	size = vswprintf(mString, size+1, fmt, args);
	va_end(args);
	return *this;
}

wchar_t& String::at(uint i)
{
	assert(i < mCapacity);
	return mString[i];
}

wchar_t& String::operator [] (uint i)
{
	assert(i < mCapacity);
	return mString[i];
}

String& String::operator = (wchar_t c)
{
	return assign(&c, 1);
}

String& String::operator = (const wchar_t* s)
{
	return assign(s);
}

String& String::operator = (const char* s)
{
	return assign(s);
}

String& String::operator = (const String& s)
{
	return assign(s);
}

String& String::operator += (wchar_t c)
{
	return append(&c, 1);
}

String& String::operator += (const wchar_t* s)
{
	return append(s);
}

String& String::operator += (const String& s)
{
	return append(s);
}

String& String::move(String& s)
{
	_printf("String::move\n");
	safe_delete_array(mString);
	mString = s.mString;
	mLength = s.mLength;
	mCapacity = s.mCapacity;
	s.mString = 0;
	s.mLength = 0;
	s.mCapacity = 0;
	return *this;
}

int compare_impl(const wchar_t* dst, const wchar_t* src, uint n)
{
	if(!dst or !src or !n) return 0;
	for(; n--; dst++, src++)
	{
		if(*dst != *src)
			return *(byte*)dst < *(byte*)src ? -1 : 1;
	}
	return 0;
}

uint wstringLength(const wchar_t* src)
{
	if(!src or *src == 0) return 0;
	wchar_t* s = (wchar_t*)src;
	for(; *s; s++);
	return (s - src);
}

uint stringLength(const char* src)
{
	if(!src or *src == 0) return 0;
	char* s = (char*)src;
	for(; *s; s++);
	return (s - src);
}

String operator + (const String& lhs, const String& rhs)
{
	String obj(lhs);
	obj.append(rhs);
	return obj;
}

String operator + (const wchar_t* lhs, const String& rhs)
{
	String obj(lhs);
	obj.append(rhs);
	return obj;
}

String operator + (const String& lhs, const wchar_t* rhs)
{
	String obj(lhs);
	obj.append(rhs);
	return obj;
}

bool operator == (const String& lhs, const String& rhs)
{
	return lhs.length() == rhs.length() and compare_impl(lhs.p(), rhs.p(), lhs.length()) == 0;
}

bool operator == (const wchar_t* lhs, const String& rhs)
{
	uint n = wstringLength(lhs);
	return n == rhs.length() and compare_impl(lhs, rhs.p(), n) == 0;
}

bool operator == (const String& lhs, const wchar_t* rhs)
{
	uint n = wstringLength(rhs);
	return lhs.length() == n and compare_impl(lhs.p(), rhs, n) == 0;
}

bool operator != (const String& lhs, const String& rhs)
{
	return !(lhs == rhs);
}

bool operator != (const wchar_t* lhs, const String& rhs)
{
	return !(lhs == rhs);
}

bool operator != (const String& lhs, const wchar_t* rhs)
{
	return !(lhs == rhs);
}

bool operator < (const String& lhs, const String& rhs)
{
	return compare_impl(lhs.p(), rhs.p(), min(lhs.length(), rhs.length()) + 1) < 0;
}

bool operator < (const wchar_t* lhs, const String& rhs)
{
	return compare_impl(lhs, rhs.p(), min(wstringLength(lhs), rhs.length()) + 1) < 0;
}

bool operator < (const String& lhs, const wchar_t* rhs)
{
	return compare_impl(lhs.p(), rhs, min(lhs.length(), wstringLength(rhs)) + 1) < 0;
}

bool operator <= (const String& lhs, const String& rhs)
{
	return compare_impl(lhs.p(), rhs.p(), min(lhs.length(), rhs.length()) + 1) <= 0;
}

bool operator <= (const wchar_t* lhs, const String& rhs)
{
	return compare_impl(lhs, rhs.p(), min(wstringLength(lhs), rhs.length()) + 1) <= 0;
}

bool operator <= (const String& lhs, const wchar_t* rhs)
{
	return compare_impl(lhs.p(), rhs, min(lhs.length(), wstringLength(rhs)) + 1) <= 0;
}

bool operator > (const String& lhs, const String& rhs)
{
	return compare_impl(lhs.p(), rhs.p(), min(lhs.length(), rhs.length()) + 1) > 0;
}

bool operator > (const wchar_t* lhs, const String& rhs)
{
	return compare_impl(lhs, rhs.p(), min(wstringLength(lhs), rhs.length()) + 1) > 0;
}

bool operator > (const String& lhs, const wchar_t* rhs)
{
	return compare_impl(lhs.p(), rhs, min(lhs.length(), wstringLength(rhs)) + 1) > 0;
}

bool operator >= (const String& lhs, const String& rhs)
{
	return compare_impl(lhs.p(), rhs.p(), min(lhs.length(), rhs.length()) + 1) >= 0;
}

bool operator >= (const wchar_t* lhs, const String& rhs)
{
	return compare_impl(lhs, rhs.p(), min(wstringLength(lhs), rhs.length()) + 1) >= 0;
}

bool operator >= (const String& lhs, const wchar_t* rhs)
{
	return compare_impl(lhs.p(), rhs, min(lhs.length(), wstringLength(rhs)) + 1) >= 0;
}

}