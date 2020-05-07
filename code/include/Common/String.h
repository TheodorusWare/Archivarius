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
#ifndef _TAH_String_h_
#define _TAH_String_h_

#include <Common/Config.h>

namespace tas
{

/// UTF-16
class TAA_LIB String
{
public:

	String();
	String(const wchar_t* s);
	String(const char* s);
	String(const String& s);
	~String();

	wchar_t* p() const;
	uint length() const;
	uint capacity() const;

	void reallocate();
	void resize(uint n); /// string
	void resize(uint n, wchar_t c); /// string
	void reserve(uint newCapacity); /// memory
	void clear();  /// set empty
	bool empty();  /// is empty ?
	uint update(); /// length

	/** Assign buffer.
	  * @param s Insertion string or buffer.
	  * @param n String or buffer length.
	  */
	String& assign(const String& s);
	String& assign(const String& s, uint p, uint n = -1);
	String& assign(const wchar_t*   s);
	String& assign(const wchar_t*   s, uint n);
	String& assign(wchar_t c);

	/** Assign UTF-8 string. */
	String& assign(const char* s);
	String& assign(const char* s, uint n);

	/** Append buffer.
	  * @param s Insertion string or buffer.
	  * @param n String or buffer length.
	  */
	String& append(const String&  s);
	String& append(const String&  s, uint p, uint n);
	String& append(const wchar_t* s);
	String& append(const wchar_t* s, uint n);
	String& append(wchar_t c);

	/** Insert string.
	  * @param pos Insertion position in current string.
	  * @param s Insertion string or buffer.
	  * @param p Insertion string position.
	  * @param n Insertion string length.
	  * @remark Before insertion, contents
	  * of current string shift up from specified position.
	  */
	String& insert(uint pos, const String& s);
	String& insert(uint pos, const String& s, uint p, uint n);
	String& insert(uint pos, const wchar_t* s, uint n);
	String& insert(uint pos, wchar_t c);

	/** Replace sequence current string by input string.
	  * @param p Position in current string.
	  * @param nr Count replace symbols in current string.
	  * @param s Input string or buffer.
	  * @param ns Count inserted symbols from input string.
	  * @return Current string reference.
	  */
	String& replace(uint p, uint nr, const String&  s);
	String& replace(uint p, uint nr, const wchar_t* s, uint ns);
	String& replace(uint p, wchar_t c);

	/** Erase symbols from current string.
	  * @param p Position in current string.
	  * @param n Count erase symbols.
	  */
	String& erase(uint p, uint n);

	/** Generate substring from current to new object. */
	String substr(uint p, uint n = -1) const;

	/** Generate substring inside current. */
	String& substrs(uint p, uint n = -1);

	/** Generate substring from current to new by separator.
	  * @param sp Separator.
	  * @param rv Reverse search by 1, else direct.
	  * @param m Mode extract, 1 after, 0 before separator.
	  */
	String substrx(wchar_t sp, wchar_t rv, wchar_t m) const;

	/** Generate substring inside current by separator. */
	String& substrxs(wchar_t sp, wchar_t rv, wchar_t m);

	/** Trim current string by separator.
	  * @param sp Separator.
	  * @param rv Reverse search by 1, else direct.
	  * @remark Found sp, string[sp] = 0.
	  */
	String& trim(wchar_t sp, wchar_t rv = 1);

	/** Trim spaces around string. */
	String& trim();

	/** Swap two string. */
	String& swap(String& s);

	/** Direct search string.
	  * @param s Search string.
	  * @param p Start search position in current string.
	  * @param n Search string length.
	  * @return Position first matching string in success case,
	  * otherwise maximum value of uint type.
	  */
	uint find(const String& s, uint p = 0) const;
	uint find(const wchar_t* s, uint p = 0) const;
	uint find(const wchar_t* s, uint p, uint n) const;
	uint find(wchar_t c, uint p = 0) const;

	/** Reverse search string, see find.
	  * @param p Position last character in current string.
	  * string where begin search.
	  * p >= length - 1 means full search.
	  */
	uint findr(const String&  s, uint p = -1) const;
	uint findr(const wchar_t* s, uint p = -1) const;
	uint findr(const wchar_t* s, uint p, uint n) const;
	uint findr(wchar_t c, uint p = -1) const;

	/** Search first not c */
	uint findn(wchar_t c, uint p = 0) const;
	/** Search last not c */
	uint findnr(wchar_t c, uint p = -1) const;

	/** String comparison.
	  * @param s Input string.
	  * @param p Input string position.
	  * @param n Input string count symbols for compare.
	  * @return Relation between strings, difference A and B.
	  * @remark Methods with argument 'n' compare by count,
	  * otherwise full strings.
	  */
	int compare(const String& s);
	int compare(const String& s, uint p, uint n);
	int compare(const wchar_t* s);
	int compare(const wchar_t* s, uint n);

	String& tolower();
	String& toupper();

	/** Convert each byte string in two byte hex representation. */
	String& tohex();

	String& format(const char*    fmt, ...);
	String& format(const wchar_t* fmt, ...);

	wchar_t&   at          (uint i);
	wchar_t&   operator [] (uint i);

	String& operator =  (wchar_t c);
	String& operator =  (const wchar_t*   s);
	String& operator =  (const char* s);
	String& operator =  (const String& s);

	String& operator += (wchar_t c);
	String& operator += (const wchar_t* s);
	String& operator += (const String& s);

	/** Append operators */
	TAA_LIB friend String operator + (const String&  lhs, const String&  rhs);
	TAA_LIB friend String operator + (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend String operator + (const String&  lhs, const wchar_t* rhs);

	/** Relational operators */
	TAA_LIB friend bool operator == (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator == (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator == (const String&  lhs, const wchar_t* rhs);
	TAA_LIB friend bool operator != (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator != (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator != (const String&  lhs, const wchar_t* rhs);
	TAA_LIB friend bool operator <  (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator <  (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator <  (const String&  lhs, const wchar_t* rhs);
	TAA_LIB friend bool operator <= (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator <= (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator <= (const String&  lhs, const wchar_t* rhs);
	TAA_LIB friend bool operator >  (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator >  (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator >  (const String&  lhs, const wchar_t* rhs);
	TAA_LIB friend bool operator >= (const String&  lhs, const String&  rhs);
	TAA_LIB friend bool operator >= (const wchar_t* lhs, const String&  rhs);
	TAA_LIB friend bool operator >= (const String&  lhs, const wchar_t* rhs);

	/** Move s in current object. */
	String& move(String& s);

private:

	wchar_t* mString; /// with terminating null-character ('\0') at the end.
	uint  mLength;
	uint  mCapacity;
};

}

#endif