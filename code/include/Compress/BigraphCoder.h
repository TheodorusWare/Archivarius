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
#ifndef _TAH_BigraphCoder_h_
#define _TAH_BigraphCoder_h_

#include <Compress/ICoder.h>

namespace tas
{

/// Replace often two bytes sequences by single not using bytes
class BigraphCoder: public ICoder
{
public:

	struct impl;
	impl* _m;

	BigraphCoder();
	~BigraphCoder();

	/** Initialisation.
	  * @param mode Process mode, 1 parsing, 2 compress, 3 uncompress.
	  * @return Positive value.
	  */
	int initialise(byte mode);

	/** Parsing, compress, uncompress memory blocks.
	  * @param sm[in, out] Source, destination data.
	  * @param flush Data to out stream.
	  * @return See return codes in ICoder.h
	  * @remark Destination size must be not less than 4 KB
	  */
	int parsing     (CoderStream* sm, byte flush);
	int compress    (CoderStream* sm, byte flush);
	int uncompress  (CoderStream* sm, byte flush);
};

}

#endif