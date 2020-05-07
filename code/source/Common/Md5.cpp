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
#include <Common/Md5.h>

#define F(x, y, z) ((x & y) | (~x & z))
#define G(x, y, z) ((x & z) | (y & ~z))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | ~z))

#define FF(a, b, c, d, k, s, i) (b + rotl((a + F(b,c,d) + loc[(k)%16] + table[i]), s))
#define GG(a, b, c, d, k, s, i) (b + rotl((a + G(b,c,d) + loc[(k)%16] + table[i]), s))
#define HH(a, b, c, d, k, s, i) (b + rotl((a + H(b,c,d) + loc[(k)%16] + table[i]), s))
#define II(a, b, c, d, k, s, i) (b + rotl((a + I(b,c,d) + loc[(k)%16] + table[i]), s))

namespace tas
{

// rotate left
static uint rotl(uint x, uint n)
{
	return x << n | x >> 32 - n;
}

Md5::Md5()
{
	uint tablep[64] =
	{
		0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
		0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
		0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
		0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
		0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
		0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
		0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
		0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
	};
	state = new uint[88];
	digest = (byte*)(state + 4);
	table = state + 8;
	forn(64) table[i] = tablep[i];
	initialise();
}

Md5::~Md5()
{
	safe_delete_array(state);
}

byte* Md5::getDigest()
{
	return digest;
}

int Md5::initialise()
{
	total = 0;
	state[0] = 0x67452301;
	state[1] = 0xEFCDAB89;
	state[2] = 0x98BADCFE;
	state[3] = 0x10325476;
	return 1;
}

byte* Md5::calculate(byte* block, uint length, byte end)
{
	total += length;

	// message buffer
	uint* loc = state + 72;
	byte* lob = (byte*)loc;

	uint mod = length % 64;
	assert(end ? 1 : mod == 0);
	uint totalBlock = length / 64;
	if(mod) totalBlock++;
	uint inputBlock = totalBlock;

	// extra block if remaining bytes less 9
	if(end and (mod == 0 or mod > 55)) totalBlock++;

	// restore md buffer
	uint &A = state[0], &B = state[1], &C = state[2], &D = state[3];
	uint AA, BB, CC, DD;

	// additional stuff
	uint k, n;
	uint* blockw = (uint*) block;

	// process message
	forn(totalBlock)
	{
		// copy bytes from input to temp
		if(i < inputBlock)
		{
			uint ln = (mod and i == inputBlock - 1) ? mod : 64;
			if(ln == 64)
			{
				loc = blockw + i * 16;
			}
			else
			{
				loc = (uint*)lob;
				form(j, ln)
				lob[j] = block[i * 64 + j];
			}
			// append nils, last block
			if(ln < 64)
			{
				ln = 64 - mod;
				form(j, ln)
				lob[mod + j] = 0;
				// append 80h
				lob[mod] = 0x80;
			}
		}
		else // extra block
		{
			loc = (uint*)lob;
			form(j, 16)
			loc[j] = 0;
			if(mod == 0)
				lob[0] = 0x80;
		}

		// append message length in bits to last 8 bytes
		if(end and i == totalBlock - 1)
		{
			total <<= 3;
			uint* ml = (uint*)&total;
			loc[14] = ml[0];
			loc[15] = ml[1];
		}

		// save md buffer
		AA = A;
		BB = B;
		CC = C;
		DD = D;

		// round 1
		form(j, 4)
		{
			k = j * 4;
			A = FF(A, B, C, D, k, 7, k);
			D = FF(D, A, B, C, k + 1, 12, k + 1);
			C = FF(C, D, A, B, k + 2, 17, k + 2);
			B = FF(B, C, D, A, k + 3, 22, k + 3);
		}

		// round 2
		form(j, 4)
		{
			k = 1 + j * 4;
			n = 16 + j * 4;
			A = GG(A, B, C, D, k, 5, n);
			D = GG(D, A, B, C, k + 5, 9, n + 1);
			C = GG(C, D, A, B, k + 10, 14, n + 2);
			B = GG(B, C, D, A, k + 15, 20, n + 3);
		}

		// round 3
		form(j, 4)
		{
			k = (5 + j * 12) % 16;
			n = 32 + j * 4;
			A = HH(A, B, C, D, k, 4, n);
			D = HH(D, A, B, C, k + 3, 11, n + 1);
			C = HH(C, D, A, B, k + 6, 16, n + 2);
			B = HH(B, C, D, A, k + 9, 23, n + 3);
		}

		// round 4
		form(j, 4)
		{
			k = (j * 12) % 16;
			n = 48 + j * 4;
			A = II(A, B, C, D, k, 6, n);
			D = II(D, A, B, C, k + 7, 10, n + 1);
			C = II(C, D, A, B, k + 14, 15, n + 2);
			B = II(B, C, D, A, k + 21, 21, n + 3);
		}

		A += AA;
		B += BB;
		C += CC;
		D += DD;
	}

	// copy states to output buffer
	uint* digestw = (uint*)digest;
	digestw[0] = A;
	digestw[1] = B;
	digestw[2] = C;
	digestw[3] = D;

	if(end) initialise();
	return digest;
}

}