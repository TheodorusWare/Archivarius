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
#ifndef _TAH_AesEncryption_h_
#define _TAH_AesEncryption_h_

#include <Common/Config.h>

namespace tas
{

/// AES 128 encryption
class TAA_LIB Aes
{
public:
	Aes();
	~Aes();

	/** Encrypt, decrypt arbitrary data size in ECB mode.
	  * @param data[in, out] Input, output data block.
	  * @param size Minimum data size 16 byte, may be not multiple 16.
	  * @param pos Block position in stream.
	  */
	void encrypt(byte* data, uint size, uint pos);
	void decrypt(byte* data, uint size, uint pos);

	/** Generate a series of round Keys from each 128 bit cipher key.
	  * @param size Size key in bytes, minimum key size 16 byte, must be multiple 16.
	  */
	void keyExpansion(byte* key, uint size);

	/** Set count of rounds.
	  * @remark Default 10 rounds.
	  */
	void setRounds(byte round);

private:

	/// encrypt, decrypt single block 16 byte, key index
	void encryptBlock(byte* block, uint keyn);
	void decryptBlock(byte* block, uint keyn);

	void addRoundKey(byte* block, byte* key);

	void subBytes(byte* block);
	void invSubBytes(byte* block);

	void shiftRows(byte* block);
	void invShiftRows(byte* block);

	void mixColumns(byte* block);
	void invMixColumns(byte* block);

	/// column transform for key expansion
	uint tmColumn(uint column, byte rcon);

	byte* sbox; /// Non-linear substitution table
	byte* rsbox; /// Inverse non-linear substitution table

	/// 8 arrays of galois polynomial column by 256 uint
	uint* ge0; /// encrypt column 0
	uint* ge1;
	uint* ge2;
	uint* ge3;

	uint* gd0; /// decrypt column 0
	uint* gd1;
	uint* gd2;
	uint* gd3;

	byte* pnAes; /// penultimate ciphertext
	byte* keyExp; /// each key expansion
	uint keyNum;
	uint keySize; /// key expansion size

	byte round; /// count 10
};

}

#endif