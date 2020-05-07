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
#include <Common/Aes.h>
#include <Common/Math.h>
#include <Common/StringLib.h>

namespace tas
{

Aes::Aes()
{
	round = 10;
	keyExp = 0;
	keyNum = 0;

	ge0 = (uint*) (new byte[8736]);
	ge1 = ge0 + 256;
	ge2 = ge1 + 256;
	ge3 = ge2 + 256;

	gd0 = ge3 + 256;
	gd1 = gd0 + 256;
	gd2 = gd1 + 256;
	gd3 = gd2 + 256;

	sbox = (byte*) (gd3 + 256);
	rsbox = sbox + 256;
	pnAes = rsbox + 256;

	// non-linear substitution table
	byte sbox_[256] =
	{
		//0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
		0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
		0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
		0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
		0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
		0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
		0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
		0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
		0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
		0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
		0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
		0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
		0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
		0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
		0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
		0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
		0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
	};

	forn(256)
	{
		sbox[i] = sbox_[i];
		rsbox[sbox[i]] = i;
	}

	// 8 tables of galois polynomial column by 256 uint
	forn(256)
	{
#define xtime(x) ((x << 1) ^ (((x & 0x80) != 0) * 0x1B))
#define UI(p0, p1, p2, p3) (p0 | p1 << 8 | p2 << 16 | p3 << 24)

		byte p1 = i;
		byte p2 = xtime(p1);
		byte p4 = xtime(p2);
		byte p8 = xtime(p4);

		byte p3  = p2 ^ p1;
		byte p09 = p8 ^ p1;
		byte p11 = p8 ^ p2 ^ p1;
		byte p13 = p8 ^ p4 ^ p1;
		byte p14 = p8 ^ p4 ^ p2;

		ge0[i] = UI(p2, p1, p1, p3);
		ge1[i] = UI(p3, p2, p1, p1);
		ge2[i] = UI(p1, p3, p2, p1);
		ge3[i] = UI(p1, p1, p3, p2);

		gd0[i] = UI(p14, p09, p13, p11);
		gd1[i] = UI(p11, p14, p09, p13);
		gd2[i] = UI(p13, p11, p14, p09);
		gd3[i] = UI(p09, p13, p11, p14);
	}
}

Aes::~Aes()
{
	safe_delete_array(ge0);
	safe_delete_array(keyExp);
}

void Aes::encrypt(byte* data, uint size, uint pos)
{
	assert(size >= 16 and keyNum != 0 and keyExp != 0);
	uint blockCount = size / 16;
	byte mod = size % 16;
	// key index
	uint keyn = (pos / 16) % keyNum;
	if(mod) // ciphertext stealing
	{
		blockCount++;
		goto aesmod;
	}

	forn(blockCount)
	{
		encryptBlock(data, keyn);
		data += 16;
		keyn = (keyn + 1) % keyNum;
	}
	return;

aesmod:
	forn(blockCount)
	{
		if(!mod or i != blockCount-1)
			encryptBlock(data, keyn);

		if(mod)
		{
			if(i == blockCount-2) // penultimate
			{
				form(u, mod)
				pnAes[u] = data[u];
			}
			else if(i == blockCount-1) // last
			{
				byte* pnd = data - 16;
				form(u, mod)
				pnd[u] = data[u];
				encryptBlock(pnd, keyn);
				// copy penultimate ciphertext to last block
				form(u, mod)
				data[u] = pnAes[u];
			}
		}
		data += 16;
		keyn = (keyn + 1) % keyNum;
	}
}

void Aes::decrypt(byte* data, uint size, uint pos)
{
	assert(size >= 16 and keyNum != 0 and keyExp != 0);
	uint blockCount = size / 16;
	byte mod = size % 16;
	// key index
	uint keyn = (pos / 16) % keyNum;
	if(mod) // ciphertext stealing
	{
		blockCount++;
		goto aesmod;
	}

	forn(blockCount)
	{
		decryptBlock(data, keyn);
		data += 16;
		keyn = (keyn + 1) % keyNum;
	}
	return;

aesmod:
	forn(blockCount)
	{
		if(mod and i == blockCount-2)
			keyn = (keyn + 1) % keyNum;

		if(!mod or i != blockCount-1)
			decryptBlock(data, keyn);

		if(mod)
		{
			if(i == blockCount-2) // penultimate, last ciphertext
			{
				form(u, mod)
				pnAes[u] = data[u];
			}
			else if(i == blockCount-1) // last, penultimate ciphertext
			{
				keyn = keyn == 0 ? keyNum-1 : keyn-1;
				byte* pnd = data - 16;
				form(u, mod)
				pnd[u] = data[u];
				decryptBlock(pnd, keyn);
				// copy penultimate ciphertext to last block
				form(u, mod)
				data[u] = pnAes[u];
			}
		}
		data += 16;
		if(!mod or i < blockCount-2)
			keyn = (keyn + 1) % keyNum;
	}
}

void Aes::encryptBlock(byte* block, uint keyn)
{
	byte* key = keyExp + keyn * keySize;

	addRoundKey(block, key);
	key += 16;

	// rounds
	forn(round)
	{
		subBytes(block);
		shiftRows(block);
		if(i < round-1)
			mixColumns(block);
		addRoundKey(block, key);
		key += 16;
	}
}

void Aes::decryptBlock(byte* block, uint keyn)
{
	byte* key = keyExp + keyn * keySize + keySize - 16;

	addRoundKey(block, key);
	key -= 16;

	// rounds
	forn(round)
	{
		invShiftRows(block);
		invSubBytes(block);
		addRoundKey(block, key);
		// if(i < 9)
		if(i < round-1)
			invMixColumns(block);
		key -= 16;
	}
}

void Aes::keyExpansion(byte* key, uint size)
{
	assert(size >= 16 and size % 16 == 0);
	keyNum = size / 16;
	keySize = (round + 1) * 16;

	// From each key block (16 bytes) generate 10 keys + input key
	// 11 * 16 = 176 bytes
	safe_delete_array(keyExp);
	keyExp = new byte[keyNum * keySize];

	// pointer to current key
	uint *kp = (uint*)keyExp;

	// round constant
	// 0x8d polynomial 1
	// 0x01 polynomial 2
	byte rcon[20] = {0};
	byte rc = 0x8D;
	forn(round)
	rcon[i] = rc = xtime(rc);

	// for each key block (16 bytes) generate 10 keys
	forn(keyNum)
	{
		// copy first input key
		mencpy(kp, key + i * 16, 16);
		kp += 4;

		// generate N keys for N rounds, N * 4 columns
		form(u, round * 4)
		{
			// u % nk
			if(u % 4 == 0)
				kp[u] = kp[u-4] ^ tmColumn(kp[u-1], rcon[u/4]);
			else
				kp[u] = kp[u-4] ^ kp[u-1];
		}
		kp += round * 4;
	}
}

void Aes::setRounds(byte roundn)
{
	round = clamp(roundn, 16, 2);
}

uint Aes::tmColumn(uint column, byte rcon)
{
	byte* lc = (byte*)&column;

	// rot word
	byte t = lc[0];
	forn(3)
	lc[i] = lc[i + 1];
	lc[3] = t;

	// sub bytes
	forn(4)
	lc[i] = sbox[lc[i]];

	// first byte xor with rcon
	lc[0] ^= rcon;

	return column;
}

void Aes::addRoundKey(byte* block_, byte* key_)
{
	uint* block = (uint*)block_;
	uint* key = (uint*)key_;
	forn(4)
	block[i] ^= key[i];
}

void Aes::subBytes(byte* block)
{
	forn(16)
	block[i] = sbox[block[i]];
}

void Aes::invSubBytes(byte* block)
{
	forn(16)
	block[i] = rsbox[block[i]];
}

void Aes::shiftRows(byte* block)
{
	// shift row 1 by 1 left
	// 1 5 9 13
	// 5 9 13 1
	byte t = block[1];
	forn(3)
	block[i * 4 + 1] = block[i * 4 + 5];
	block[13] = t;

	// shift row 2 by 2
	// 2  6  10 14
	// 10 14 2  6
	t = block[2];
	block[2] = block[10];
	block[10] = t;
	t = block[6];
	block[6] = block[14];
	block[14] = t;

	// shift row 3 by 3 (to right by 1)
	// 3 7 11 15
	// 15 3 7 11
	t = block[15];
	forn(3)
	block[15 - i * 4] = block[11 - i * 4];
	block[3] = t;
}

void Aes::invShiftRows(byte* block)
{
	// shift row 1 by 1 right
	// 1 5 9 13
	// 13 1 5 9
	byte t = block[13];
	forn(3)
	block[13 - i * 4] = block[9 - i * 4];
	block[1] = t;

	// shift row 2 by 2
	// 2  6  10 14
	// 10 14 2  6
	t = block[2];
	block[2] = block[10];
	block[10] = t;
	t = block[6];
	block[6] = block[14];
	block[14] = t;

	// shift row 3 by 3 (to left by 1)
	// 3 7 11 15
	// 7 11 15 3
	t = block[3];
	forn(3)
	block[i * 4 + 3] = block[i * 4 + 7];
	block[15] = t;
}

void Aes::mixColumns(byte* block)
{
	// each column
	byte* c = block;
	uint* v = (uint*)block;
	forn(4)
	{
		*v = ge0[c[0]] ^ ge1[c[1]] ^ ge2[c[2]] ^ ge3[c[3]];
		c += 4;
		v++;
	}
}

void Aes::invMixColumns(byte* block)
{
	// each column
	byte* c = block;
	uint* v = (uint*)block;
	forn(4)
	{
		*v = gd0[c[0]] ^ gd1[c[1]] ^ gd2[c[2]] ^ gd3[c[3]];
		c += 4;
		v++;
	}
}

}
