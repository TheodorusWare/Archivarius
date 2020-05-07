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
#ifndef _TAH_FileStream_h_
#define _TAH_FileStream_h_

#include <Common/Config.h>
#include <Common/String.h>

namespace tas
{

/// Binary file stream for read, write.
class TAA_LIB FileStream
{
public:

	FileStream(const String& fname, bool load, const String& mode = "");
	FileStream();
	~FileStream();

	bool open(const String& fname, bool load, const String& mode = "");
	void close();

	/// FILE* fp
	void    setf(void* fp, bool close);
	void*   getf();

	String  getName();

	bool    state();
	wint    size();  /// file size bytes
	void    begin(); /// go to begin file

	bool    seek(int offset, int origin);
	int     tell();
	bool    seekw(int64 offset, int origin);
	wint    tellw();

	byte    readByte();
	half    readHalf();
	uint    readUint();
	wint    readWint();
	int     readInt();
	float   readFloat();
	double  readDouble();
	uint	  readBuffer(void* buffer, uint size);

	void    storeByte(byte v);
	void    storeHalf(half v);
	void    storeUint(uint v);
	void    storeWint(wint v);
	void    storeInt(int v);
	void    storeFloat(float v);
	void    storeDouble(double v);
	uint	  storeBuffer(void* buffer, uint size);

	bool    eof();
	void    save();
	uint    grwb(); /// get rwb
	void    setbuf(char* buf, int mode, uint size);

private:
	struct impl;
	impl* m;
	String fname;
	uint  rwb; /// read / write bytes
	bool closf;
};

}

#endif