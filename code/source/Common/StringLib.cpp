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
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#endif
#include <Common/StringLib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

namespace tas
{

uint stdlen(char* src)
{
	if(!src or *src == 0) return 0;
	char* s = 0;
	for(s = src; *s; s++);
	return (s - src);
}

uint stwlen(wchar_t* src)
{
	if(!src or *src == 0) return 0;
	wchar_t* s = 0;
	for(s = src; *s; s++);
	return (s - src);
}

char* stdcpy(char* dst, char* src)
{
	if(!dst or !src) return dst;
	char* s = dst;
	while((*s++ = *src++) != 0);
	return dst;
}

wchar_t* stwcpy(wchar_t* dst, wchar_t* src)
{
	if(!dst or !src) return dst;
	wchar_t* s = dst;
	while((*s++ = *src++) != 0);
	return dst;
}

char* stdncpy(char *dst, char *src, uint len)
{
	if(!dst or !src or !len) return dst;
	char* s = dst;
	for(; len and (*s = *src); len--, s++, src++);
	while(len--) *s++ = 0;
	return dst;
}

char* stdcpyf(char* dst, char* fmt, ...)
{
	if(!dst or !stdlen(fmt)) return dst;
	va_list args;
	va_start(args, fmt);
	vsprintf(dst, fmt, args);
	va_end(args);
	return dst;
}

char* stdcat(char* dst, char* src)
{
	if(!dst or !stdlen(src)) return dst;
	stdcpy(dst + stdlen(dst), src);
	return dst;
}

char* stdncat(char* dst, char* src, uint n)
{
	if(!dst or !stdlen(src) or !n) return dst;
	stdncpy(dst + stdlen(dst), src, n);
	return dst;
}

bool stdcmp(char* dst, char* src, bool noCase)
{
	if(!dst or !src) return 0;
	char d, s;
	while(*src != 0)
	{
		d = noCase ? stdlwc(*dst) : *dst;
		s = noCase ? stdlwc(*src) : *src;
		if(d != s) break;
		dst++;
		src++;
	}
	return (*dst == *src);
}

bool stdncmp(char* dst, char* src, uint n, bool noCase)
{
	if(!dst or !src or !n) return 0;
	char d, s;
	while(n and *src != 0)
	{
		d = noCase ? stdlwc(*dst) : *dst;
		s = noCase ? stdlwc(*src) : *src;
		if(d != s) break;
		dst++;
		src++;
		n--;
	}
	return !n;
}

int stdcmpi(char* dst, char* src, bool noCase)
{
	if(!dst or !src) return 0;
	byte d, s;
	while(1)
	{
		d = noCase ? stdlwc(*dst) : *dst;
		s = noCase ? stdlwc(*src) : *src;
		if(d == '\0' or d != s)
			return d == s ? 0 : (d < s ? -1 : 1);
		dst++;
		src++;
	}
	return 0;
}

int stdncmpi(char* dst, char* src, uint n, bool noCase)
{
	if(!dst or !src or !n) return 0;
	byte d, s;
	while(n--)
	{
		d = noCase ? stdlwc(*dst) : *dst;
		s = noCase ? stdlwc(*src) : *src;
		if(d == '\0' or d != s)
			return d < s ? -1 : 1;
		dst++;
		src++;
	}
	return 0;
}

bool stdpat(char* src, char* pat)
{
	if(!src or !pat) return 0;
	for (; *pat != '\0'; pat++)
	{
		switch (*pat)
		{
		case '?':
			++src;
			break;
		case '*':
		{
			int max = stdlen(src);
			if (pat[1] == '\0' or max == 0)
				return true;
			for (int i = 0; i < max; i++)
				if (stdpat(src + i, pat+1))
					return true;
			return false;
		}
		default:
			if (*src != *pat)
				return false;
			++src;
		}
	}
	return *src == '\0';
}

bool stwpat(wchar_t* src, wchar_t* pat)
{
	if(!src or !pat) return 0;
	for (; *pat != '\0'; pat++)
	{
		switch (*pat)
		{
		case '?':
			++src;
			break;
		case '*':
		{
			int max = stwlen(src);
			if (pat[1] == '\0' or max == 0)
				return true;
			for (int i = 0; i < max; i++)
				if (stwpat(src + i, pat+1))
					return true;
			return false;
		}
		default:
			if (*src != *pat)
				return false;
			++src;
		}
	}
	return *src == '\0';
}

int stdchr(char* src, char c)
{
	if(!src) return -1;
	for(char* s = src; *s; s++)
		if(*s == c)
			return s - src;
	return -1;
}

int stdrchr(char* src, char c)
{
	if(!src) return -1;
	int p = -1;
	for(char* s = src; *s; s++)
	{
		if(*s == c)
			p = s - src;
	}
	return p;
}

int stdstr(char* src, char* pat)
{
	if(!src or !pat) return -1;
	for(char* s = src; *s; s++)
	{
		if(*s == *pat)
		{
			char* a = s;
			char* p = pat;
			while(*a != 0 and *a == *p)
			{
				a++;
				p++;
			}
			if(*p == 0) return s - src;
		}
	}
	return -1;
}

int stdrstr(char* src, char* pat)
{
	if(!src or !pat) return -1;
	int n = -1;
	for(char* s = src; *s; s++)
	{
		if(*s == *pat)
		{
			char* a = s;
			char* p = pat;
			while(*a != 0 and *a == *p)
			{
				a++;
				p++;
			}
			if(*p == 0) n = s - src;
		}
	}
	return n;
}

// insert before p
char* stdins(char* dst, char c, uint p)
{
	if(!dst) return dst;
	uint n = stdlen(dst);
	forn(n - p)
	dst[n - i] = dst[n - i - 1];
	dst[p] = c;
	dst[n + 1] = 0;
	return dst;
}

// insert before p
char* stdins(char* dst, char* ins, uint p)
{
	uint l = stdlen(ins);
	if(!dst or !l) return dst;
	uint n = stdlen(dst);
	if(p > n) return dst;
	uint nc = n + l - 1;
	forn(n - p)
	dst[nc - i] = dst[n - i - 1];
	forn(l)
	dst[p + i] = ins[i];
	dst[nc + 1] = 0;
	return dst;
}

// generate ABCD-123
// ns - num chars, nd - num digits, uq - unique count, rv - rand val, sp - separator, lc - lower case
char* stdgen(char* dst, uint ns, uint nd, char uq, uint rv, char sp, bool lc)
{
	if(!dst) return 0;
	uint n = 0;
	uint m = 0;
	uint s = lc ? 97 : 65;
	char* d = dst;
	char* g = 0;
	if(uq > 20) uq = 20;
	forn(ns)
	{
		while(1)
		{
			rv = rv * 1664625 + 103515245;
			m = (rv % 26000) / 1000 + s;
			n = d - dst; // count
			if(n > uq) n = uq;
			if(!n) break;
			g = d - 1;
			while(n and *g-- != m) n--;
			if(!n) break; // unique
		}
		*d++ = m;
	}
	if(sp) *d++ = sp;
	if(uq > 9) uq = 9;
	forn(nd)
	{
		while(1)
		{
			rv = rv * 1664625 + 103515245;
			m = (rv % 10000) / 1000 + 48;
			n = d - dst;
			if(n > uq) n = uq;
			if(!n) break;
			g = d - 1;
			while(n and *g-- != m) n--;
			if(!n) break; // unique
		}
		*d++ = m;
	}
	*d = 0;
	return dst;
}

wchar_t* stwgen(wchar_t* dst, uint ns, uint nd, wchar_t uq, uint rv, wchar_t sp, bool lc)
{
	if(!dst) return 0;
	uint n = 0;
	uint m = 0;
	uint s = lc ? 97 : 65;
	wchar_t* d = dst;
	wchar_t* g = 0;
	if(uq > 20) uq = 20;
	forn(ns)
	{
		while(1)
		{
			rv = rv * 1664625 + 103515245;
			m = (rv % 26000) / 1000 + s;
			n = d - dst; // count
			if(n > uq) n = uq;
			if(!n) break;
			g = d - 1;
			while(n and *g-- != m) n--;
			if(!n) break; // unique
		}
		*d++ = m;
	}
	if(sp) *d++ = sp;
	if(uq > 9) uq = 9;
	forn(nd)
	{
		while(1)
		{
			rv = rv * 1664625 + 103515245;
			m = (rv % 10000) / 1000 + 48;
			n = d - dst;
			if(n > uq) n = uq;
			if(!n) break;
			g = d - 1;
			while(n and *g-- != m) n--;
			if(!n) break; // unique
		}
		*d++ = m;
	}
	*d = 0;
	return dst;
}

// extract
// sp separator, rv reverse search, m 0 extract before separator, 1 after
char* stdext(char* dst, char* src, char sp, char rv, char m)
{
	if(!dst or !stdlen(src)) return 0;
	int p = 0;
	if(rv) p = stdrchr(src, sp);
	else p = stdchr(src, sp);
	if(p != -1)
	{
		if(m) stdcpy(dst, src + p + 1);
		else stdncpy(dst, src, p);
	}
	else
		stdcpy(dst, src);
	return dst;
}

char* stdlwc(char* src)
{
	if(!src) return src;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	return CharLowerA(src);
#else
	char* s = src;
	while(*s)
	{
		if(*s >= 'A' and *s <= 'Z')
			*s += 32;
		s++;
	}
#endif
	return src;
}

char stdlwc(char c)
{
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	char s[2] = {c, 0};
	CharLowerA(s);
	return s[0];
#else
	if(c >= 'A' and c <= 'Z')
		c += 32;
#endif
	return c;
}

char* stdupc(char* src)
{
	if(!src) return src;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	return CharUpperA(src);
#else
	char* s = src;
	while(*s)
	{
		if(*s >= 'a' and *s <= 'z')
			*s -= 32;
		s++;
	}
#endif
	return src;
}

char stdupc(char c)
{
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	char s[2] = {c, 0};
	CharUpperA(s);
	return s[0];
#else
	if(c >= 'a' and c <= 'z')
		c -= 32;
#endif
	return c;
}

/// @param type 0 hour:min:sec, 1 day.mon.year
char* stdtime(char* dst, char type)
{
	if(!dst) return 0;
// #if TAA_PLATFORM == TAA_PLATFORM_LINUX
	time_t sc = time(0);
	// tm ct;
	// localtime_r(&sc, &ct);
	tm* ct = localtime(&sc);
	ct->tm_mon  += 1;
	ct->tm_year -= 100;
	char* d = dst;
	uint dt[6] = {ct->tm_hour, ct->tm_min, ct->tm_sec, ct->tm_mday, ct->tm_mon, ct->tm_year};
	uint* dc = dt;
	if(type) dc += 3;
	forn(3)
	{
		*d++ = 48 + dc[i] / 10;
		*d++ = 48 + (dc[i] % 10);
		if(i < 2) *d++ = type ? '.' : ':';
	}
	*d = 0;
// #endif
	return dst;
}

void stdprintf(char* fmt, ...)
{
	static char buffer[0x400];
	va_list args;
	va_start(args, fmt);
#ifdef TAA_VSNPRINTF
	vsnprintf(buffer, 0x400, fmt, args);
	// uint size = vsnprintf(0, 0, fmt, args);
#else
	vsprintf(buffer, fmt, args);
#endif
	va_end(args);
	printf("%s", buffer);
}

void stdfprintf(char* file, char* fmt, ...)
{
	va_list args;
	va_start (args, fmt);
// #ifdef TAA_VSNPRINTF
	// uint size = vsnprintf(NULL, 0, fmt, args);
	// char* buffer = new char[size + 2];
// #else
	static char buffer[0x400];
// #endif
	vsprintf(buffer, fmt, args);
	va_end(args);
	FILE* fileo = fopen(file, "a");
	fprintf(fileo, buffer);
	fclose(fileo);
#ifdef TAA_VSNPRINTF
	// safe_delete_array(buffer);
#endif
}

// base 2, 8, 10, 16
char* stditoa(char* src, int val, int base)
{
	char* out = src;
	bool neg = 0;
	int rem = 0;

	if(val == 0)
	{
		*out++ = '0';
		*out = 0;
		return src;
	}

	if(val < 0 and base == 10)
	{
		neg = 1;
		val = -val;
	}

	while(val != 0)
	{
		rem = val % base;
		*out++ = (rem > 9)? (rem - 10) + 'A' : rem + '0';
		val /= base;
	}

	if(neg)
		*out++ = '-';

	*out = 0;

	// reverse
	char* s = src;
	char* e = out - 1;
	char  k = 0;

	while(s < e)
	{
		k = *s;
		*s++ = *e;
		*e-- = k;
	}

	return src;
}

char* stdftoa(char* src, float val, int prec)
{
	bool neg = 0;
	if(val < 0.0)
	{
		neg = 1;
		val = -val;
	}

	char* out = src;

	// limit precision to 9, cause a prec >= 10 can lead to overflow errors
	while(prec > 9)
	{
		*out++ = '0';
		prec--;
	}

	double pwr = 1.0;
	// power 10^n
	if(prec > 0)
	{
		forn(prec)
		pwr *= 10.0;
	}

	int rem = 0;
	int ival = (int) val; // integer part
	double ffrac = (val - ival) * pwr; // fractional part
	uint frac = ffrac;
	double diff = ffrac - frac;

	if(diff > 0.5)
	{
		// _line; _printfv(diff);
		++frac;
		// handle rollover, e.g. case 0.99 with prec 1 is 1.0
		if(frac >= pwr)
		{
			frac = 0;
			++ival;
		}
	}
	else if(diff < 0.5);
	else if((frac == 0U) || (frac & 1U))
	{
		// if halfway, round up if odd OR if last digit is 0
		++frac;
	}

	if(!prec)
	{
		diff = val - (double)ival;
		if ((!(diff < 0.5) || (diff > 0.5)) && (ival & 1))
		{
			// exactly 0.5 and ODD, then round up
			// 1.5 -> 2, but 2.5 -> 2
			++ival;
		}
	}

	/*    Algorithm
		1. fractional part
		2. integer part
		3. sign
		4. reverse
	*/

	// step 1, 2
	forn(2)
	{
		if(!i and !prec)
		{
			frac = ival;
			continue;
		}
		if(i and !frac)
		{
			*out++ = '0';
			continue;
		}
		while(frac != 0)
		{
			rem = frac % 10;
			*out++ = rem + '0';
			frac /= 10;
		}
		if(!i and prec)
		{
			while(out - src < prec)
				*out++ = '0';
			*out++ = '.';
		}
		frac = ival;
	}

	if(neg)
		*out++ = '-';

	*out = 0;

	// reverse
	char* s = src;
	char* e = out - 1;
	char  k = 0;

	while(s < e)
	{
		k = *s;
		*s++ = *e;
		*e-- = k;
	}

	return src;
}

int stdatoi(char* src)
{
	if(!src) return 0;
	bool neg = 0;
	char* s = src;
	if(*s == '-' or *s == '+')
	{
		if(*s == '-')
			neg = 1;
		s++;
		if(*s == 0) return 0;
	}
	int res = 0;
	while(*s)
	{
		if(*s >= 0x30 and *s <= 0x39)
			res = res * 10 + *s - 48;
		s++;
	}
	if(neg) res *= -1;
	return res;
}

int stdwtoi(wchar_t* src)
{
	if(!src) return 0;
	bool neg = 0;
	wchar_t* s = src;
	if(*s == '-' or *s == '+')
	{
		if(*s == '-')
			neg = 1;
		s++;
		if(*s == 0) return 0;
	}
	int res = 0;
	while(*s)
	{
		if(*s >= 0x30 and *s <= 0x39)
			res = res * 10 + *s - 48;
		s++;
	}
	if(neg) res *= -1;
	return res;
}

float stdatof(char* src)
{
	if(!src or !stdlen(src)) return 0;
	bool neg = 0;
	char* s = src;
	if(*s == '-' or *s == '+')
	{
		if(*s == '-')
			neg = 1;
		s++;
		if(*s == 0) return 0;
	}
	int   x = 0;
	float w = 0;
	float f = 1.0f;
	int   d = -1;
	while(*s)
	{
		if(*s >= 0x30 and *s <= 0x39)
		{
			x = x * 10 + *s - 48;
			if(d != -1)
				f *= 10.0f;
		}
		else if(*s == 0x2E)
			d = 1;
		s++;
	}
	w = (float)x / f;
	if(neg) w *= -1.0f;
	return w;
}

char* stmcpy(chars& dst, char* src)
{
	if(!src)
	{
		safe_delete_array(dst);
		return dst;
	}
	uint len = stdlen(src);
	if(!dst or len >= stdlen(dst))
	{
		safe_delete_array(dst);
#ifdef TAA_ALLOCATOR
		allocatorSrc(__FILE__,__LINE__,0,0);
#endif
		dst = new char[len + 1];
	}
	return stdcpy(dst, src);
}

char* stmncpy(chars& dst, char* src, uint n)
{
	if(!src and !n)
	{
		safe_delete_array(dst);
		return dst;
	}
	if(!src and n)
	{
		safe_delete_array(dst);
		dst = new char[n];
		dst[0] = 0;
		return dst;
	}
	if(!src or !n) return dst;
	uint l = stdlen(src);
	if(n > l) n = l;
	if(!dst or n >= stdlen(dst))
	{
		safe_delete_array(dst);
		dst = new char[n + 1];
	}
	return stdncpy(dst, src, n);
}

char* stmcpyf(chars& dst, char* fmt, ...)
{
	if(!stdlen(fmt)) return dst;
	va_list args;
	va_start(args, fmt);
#ifdef TAA_VSNPRINTF
	uint size = vsnprintf(NULL, 0, fmt, args);
#else
	uint size = 256;
#endif
	if(!dst or size > stdlen(dst))
	{
		safe_delete_array(dst);
		dst = new char[size + 1];
	}
	vsprintf(dst, fmt, args);
	va_end(args);
	return dst;
}

char* stmcat(chars& dst, char* src)
{
	if(!src) return dst;
	if(!dst) return stmcpy(dst, src);
	uint memsz = stdlen(dst) + stdlen(src) + 1;
	char* temp = new char[memsz];
	stdcpy(temp, dst);
	stdcat(temp, src);
	safe_delete_array(dst);
	dst = temp;
	return dst;
}

char* stmncat(chars& dst, char* src, uint n)
{
	if(!src or !n) return dst;
	if(!dst) return stmncpy(dst, src, n);
	uint l = stdlen(src);
	if(n > l) n = l;
	uint memsz = stdlen(dst) + n + 1;
	char* temp = new char[memsz];
	stdcpy(temp, dst);
	stdncat(temp, src, n);
	safe_delete_array(dst);
	dst = temp;
	return dst;
}

char* stmins(chars& src, char* ins, uint p)
{
	if(!src) return 0;
	uint m = stdlen(src);
	uint n = stdlen(ins);
	char* dst = new char[n + m + 1];
	stdcpy(dst, src);
	safe_delete_array(src);
	src = dst;
	stdins(src, ins, p);
	return src;
}

char* stmgen(chars& dst, uint ns, uint nd, char uq, uint rv, char sp, bool lc)
{
	uint n = ns + nd + (sp ? 1 : 0);
	if(!dst or n > stdlen(dst))
	{
		safe_delete_array(dst);
		dst = new char[n + 1];
	}
	return stdgen(dst, ns, nd, uq, rv, sp, lc);
}

char* stmext(chars& dst, char* src, char sp, char rv, char m)
{
	if(!stdlen(src)) return 0;
	int p = 0;
	if(rv) p = stdrchr(src, sp);
	else p = stdchr(src, sp);
	if(p != -1)
	{
		if(m) stmcpy(dst, src + p + 1);
		else stmncpy(dst, src, p);
	}
	else
		stmcpy(dst, src);
	return dst;
}

char* stmcpy(char* src)
{
	if(!src) return 0;
	char* dst = 0;
	return stmcpy(dst, src);
}

char* stmncpy(char* src, uint n)
{
	if(!src and n)
		return new char[n];
	if(!src or !n) return 0;
	char* dst = 0;
	return stmcpy(dst, src);
}

char* stmgen(uint ns, uint nd, char uq, uint rv, char sp, bool lc)
{
	char* dst = 0;
	return stmgen(dst, ns, nd, uq, rv, sp, lc);
}

void* mencpy(void* dst, void* src, uint n)
{
	if(!n or !dst or !src or dst == src) return dst;

	byte ws = sizeof(int);
	uint w = n / ws;
	byte m = n % ws;

	// copy 4 bytes
	if(w)
	{
		uint* d = (uint*) dst;
		uint* s = (uint*) src;
		forn(w) *d++ = *s++;
	}

	// copy bytes
	if(m)
	{
		byte* d = ((byte*) dst) + n - m;
		byte* s = ((byte*) src) + n - m;
		forn(m) *d++ = *s++;
	}
	return dst;
}

void* menmove(void* dst, void* src, uint n)
{
	if(!n or !dst or !src or dst == src) return dst;

	// front to back
	if(dst < src)
		mencpy(dst, src, n);
	else
	{
		// back to front
		byte ws = sizeof(int);
		uint w = n / ws;
		byte m = n % ws;

		// copy 4 bytes
		if(w)
		{
			uint* d = (uint*) ((byte*)dst + n - ws);
			uint* s = (uint*) ((byte*)src + n - ws);
			forn(w) *d-- = *s--;
		}
		// copy bytes
		if(m)
		{
			byte* d = ((byte*) dst) + n;
			byte* s = ((byte*) src) + n;
			forn(m) *d-- = *s--;
		}
	}

	return dst;
}

void* menset(void* dst, int value, uint n)
{
	if(!n or !dst) return dst;

	byte* d = (byte*) dst;
	byte val = value;

	forn(n)
	*d++ = val;

	return dst;
}

void* stmalloc(uint size)
{
	uint* p = (uint*) (new char[size + sizeof(uint)]);
	*p = size;
	p++;
	return p;
}

void stmfree(void* ptr)
{
	char* p = (char*) (((uint*)ptr)-1);
	safe_delete_array(p);
}

uint stmsize(void* ptr)
{
	/** cast to uint*
	  * substract 1
	  * dereference pointer
	  */
	return *(((uint*)ptr)-1);
}

void* stmrealloc(void* ptr, uint size)
{
	if(ptr == 0)
		return stmalloc(size);

	char* p = (char*) ptr;
	uint csize = stmsize(p);

	if(size <= csize)
		return ptr;

	char* pn = (char*) stmalloc(size);
	mencpy(pn, p, csize);
	p -= sizeof(uint);
	safe_delete_array(p);
	return pn;
}

void* stmrealloc(void* ptr, uint csize, uint nsize)
{
	if(ptr == 0)
		return new char[nsize];

	char* p = (char*) ptr;

	if(nsize <= csize)
		return ptr;

	char* pn = (char*) new char[nsize];
	if(csize) mencpy(pn, p, csize);
	safe_delete_array(p);
	return pn;
}

int stdfname(char* src)
{
	if(!src) return -1;
	int p = stdrchr(src, '/');
	if(p == -1) p = stdrchr(src, '\\');
	return p;
}

}