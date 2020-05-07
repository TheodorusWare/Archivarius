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
#ifndef _TAH_CmCoder_h_
#define _TAH_CmCoder_h_

#include <Compress/ICoder.h>

namespace tas
{

struct CmParameters;

/// Context mixing
class CmCoder: public ICoder
{
public:

	struct impl;
	impl* _m;

	CmCoder();
	~CmCoder();

	/** CM initialisation.
	  * @param params Coder parameters, see CmParameters.
	  *    mode Process mode, 1 compress, 2 uncompress.
	  *    dictionary[in, out] Hash table size [64 kb, 16 mb].
	  *    context[in, out] Context length [4, 8].
	  *    level Level compression, speed [1, 9] quality.
	  * @return Positive value.
	  */
	int initialise(CmParameters* params);

	/** CM compress, uncompress memory blocks.
	  * @param sm[in, out] Source, destination data.
	  * @param flush Data to out stream.
	  * @return See return codes in ICoder.h.
	  * @remark Destination size must be not less than 4 KB.
	  */
	int compress   (CoderStream* sm, byte flush);
	int uncompress (CoderStream* sm, byte flush);
};

struct CmParameters
{
	CmParameters();
	byte mode;
	uint dictionary;
	byte context;
	byte level;
};

}

#endif