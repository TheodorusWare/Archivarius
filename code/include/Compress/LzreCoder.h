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
#ifndef _TAH_LzreCoder_h_
#define _TAH_LzreCoder_h_

#include <Compress/ICoder.h>

namespace tas
{

struct LzreParameters;

/// Lempel Ziv Range Encoding
class LzreCoder: public ICoder
{
public:

	struct impl;
	impl* _m;

	LzreCoder();
	~LzreCoder();

	/** LZRE initialisation.
	  * @param params Coder parameters, see LzreParameters.
	  *    mode Process mode, 1 compress, 2 uncompress.
	  *    dictionary[in, out] Dictionary size [64 kb, 256 mb].
	  *    minMatch[in, out] Minimum match length [2, 8].
	  *    maxMatch[in, out] Maximum match length [32, 1024].
	  *    level Level compression, speed [1, 9] quality.
	  *    cycles Sets number of cycles for match finder [1, 2048].
	  *    rolz Sets number of rolz for match finder [16, 256].
	  *    suffix Maximum count suffix nodes in search tree [1, 128].
	  *    threads[in, out] Compression threads number [1, 2].
	  *    cmix Context mixing state, enable by 1, else disable.
	  * @return Positive value.
	  */
	int initialise(LzreParameters* params);

	/** LZRE compress, uncompress memory blocks.
	  * @param sm[in, out] Source, destination data.
	  * @param flush Data to out stream.
	  * @return See return codes in ICoder.h.
	  * @remark Destination size must be not less than 4 KB.
	  */
	int compress   (CoderStream* sm, byte flush);
	int uncompress (CoderStream* sm, byte flush);
};

/// LZRE coder parameters
struct LzreParameters
{
	LzreParameters();
	byte mode;
	uint dictionary;
	byte minMatch;
	half maxMatch;
	byte level;
	half cycles;
	half rolz;
	byte suffix;
	byte threads;
	byte cmix;
};

}

#endif