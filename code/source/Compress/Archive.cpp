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
#include <Compress/Archive.h>
#include <Common/FileStream.h>
#include <Common/TextStream.h>
#include <Common/ConvertBytes.h>
#include <Common/Bitwise.h>
#include <Common/Crc32.h>
#include <Common/StringLib.h>
#include <Common/UString.h>
#include <Common/Timer.h>
#include <Common/FileAccess.h>
#include <Common/Aes.h>
#include <Common/Math.h>
#include <Compress/ICoder.h>
#include <Common/container/Array.h>
#include <Common/container/SmartPtr.h>
#ifdef TAA_ARCHIVARIUS_COMPRESS
#include <Compress/BigraphCoder.h>
#include <Compress/LzreCoder.h>
#include <Compress/CmCoder.h>
#endif

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#include <Psapi.h>
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <stdio.h>
#include <time.h>
#include <Common/platform/assert.h>

namespace tas
{

// signature arv3, little endian, 4 byte
#define ARCHIVE_SIGNATURE 0x33767261

// memory block size for reading, writing
#define BLOCK_SIZE 65536

#pragma pack(push,1)
struct ArchiveHeader
{
	uint signature;   // with version (last 4 byte)
	uint filesCount;  // count files in archive
	wint dataSize;  // data size
	wint procSize;  // preprocessing size
	uint encodeProps; // compression properties
	uint options;     // low 2 bits encryption
	uint crcCrypt;    // crypt string
	uint crcHeader;   // archive header, end
};
struct ArchiveFileHeader
{
	half nameLen;
	wint size; // uncompressed
	uint crc;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	uint time[2]; // last modified time
#endif
};
#pragma pack(pop)
struct ArchiveFileList
{
	String name;
	wint size; // uncompressed
	wint pos;
	uint crc;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	uint time[2]; // last modified time
#endif
	ArchiveFileList()
	{
		size = 0;
		pos = 0;
		crc = 0;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		time[0] = 0;
		time[1] = 0;
#endif
	}
	~ArchiveFileList()
	{
	}
};

struct Archive::impl
{
	byte errorId;
	String errorStr;

	FileStream archive;
	ArchiveHeader arcHead;
	Array<ArchiveFileList*> fileList;
	Array<uint>* cutPaths;
	TextStream* log;
	Aes encryption;
	wint archiveSize;
	byte archiveHeadSize;
	byte archiveHeadSizeTotal;
	byte archiveFileHeadSize;
	wint sourceFileSize;
	uint totalFileHeadSize;
	uint dataPos;
	byte mode; // 0 nothing 1 create 2 open

	uint encodeMethod;
	uint encodeLevel;
	uint encodeThreads;
	uint encodeBigraph;
	uint encodeCmix;
	uint dictSize;
	uint matchMax;
	uint matchCycles;
	uint matchMin;
	uint matchRolz;
	uint encodeSuffix; // [11]

	uint matchRolzOut;

	uint paramsOpen[11];
	uint paramsCur[11];

	wint bigraphSize; // file
	byte deleteState;

	String decFile; // uncompressed file
	String decPath; // extract dir
	String lastFile;
	String pathArchive; // path in archive, append mode
	String buffer; // temp 1024
	UString bufferu; // temp 1024
	byte savePath;
	byte pathDelete;
	byte extractMode; // 0 - list, 1 - one file
	uint skipFileLen;
	uint ratioIn;
	uint ratioUn;

	byte   cryptState; // 0 off 1 names 2 data 3 all
	UString cryptString;
	UString cryptStringSave;
	uint   cryptLength; // key
	byte*  cryptBuffer; // 512 byte
	uint   cryptBufferSize;
	Crc32  crc;

	String format;
	String formated;

	byte* mem;
	byte* memEnc;

	Timer timer;
	uint msecondsp;
	uint mseconds;
	uint seconds;
	uint minutes;

	int open(const String& name, byte mode = 0);
	int close(byte mode = 0);
	int extract(void* files);
	int append(StringArray& files);
	int update(StringArray& files);
	int remove(StringArray& files);
	int rename(StringArray& files);
	int list();
	int checkCrypt();

	int sortFileExt(StringArray& filesIn, StringArray& filesOut);
	void logTime(uint filesCount);
	ICoder* createMethod(byte mode);
	int getTime();
	char* getUnicode(const String& wides);
	int logWriteLinef(const char* fmt, ...);

	int (*event_callback)(wint* data);
};

Archive::Archive()
{
	_m = new impl;
	_m->errorId = 0;
	_m->cutPaths = 0;
	_m->log = 0;
	_m->archiveSize = 0;
	_m->archiveHeadSizeTotal = 0;
	_m->archiveHeadSize = sizeof(ArchiveHeader);
	_m->archiveFileHeadSize = sizeof(ArchiveFileHeader);
	_m->sourceFileSize = 0;
	_m->totalFileHeadSize = 0;
	_m->dataPos = 0;
	_m->mode = 0;
	_m->encodeMethod = 0;
	_m->encodeLevel = 0;
	_m->encodeThreads = 0;
	_m->encodeBigraph = 0;
	_m->encodeCmix = 0;
	_m->bigraphSize = 0;
	_m->deleteState = 0;
	_m->dictSize = 0;
	_m->matchMax = 0;
	_m->matchCycles = 0;
	_m->matchMin = 0;
	_m->matchRolz = 0;
	_m->matchRolzOut = 0;
	_m->encodeSuffix = 0;
	_m->cryptState = 0;
	_m->cryptLength = 0;
	_m->event_callback = 0;
	_m->savePath = 0;
	_m->pathDelete = 0;
	_m->extractMode = 0;
	_m->skipFileLen = 0;
	_m->ratioIn = 0;
	_m->ratioUn = 0;
	_m->mem = 0;
	_m->memEnc = 0;
	_m->buffer.reserve(1024);
	_m->bufferu.reserve(1024);
	_m->format.reserve(1024);
	_m->formated.reserve(1024);
	_m->cryptBuffer = new byte[512];
	_m->cryptBufferSize = 0;
	_m->crc.tableInit();
	_m->msecondsp = 0;
	_m->mseconds = 0;
	_m->seconds = 0;
	_m->minutes = 0;
}

Archive::~Archive()
{
	close();
	safe_delete_array(_m->mem);
	safe_delete_array(_m->memEnc);
	safe_delete_array(_m->cryptBuffer);
	safe_delete(_m);
}

int Archive::create(const String& name)
{
	createFolder(name);
	if(!_m->archive.open(name, 0))
		return 0;
	_m->mode = 1;
	return 1;
}

int Archive::open(const String& name, byte modew)
{
	if(_m->mode == 2)
		close();
	return _m->open(name, modew);
}

int Archive::close(byte smode)
{
	return _m->close(smode);
}

int Archive::checkCrypt()
{
	return _m->checkCrypt();
}

int Archive::saveCrypt()
{
	uint length = _m->cryptString.length();
	_m->cryptStringSave.clear();
	_m->cryptStringSave.resize(length + 32);
	_m->cryptStringSave = _m->cryptString;
	return 1;
}

int Archive::resetCrypt()
{
	_m->cryptString.clear();
	_m->cryptLength = 0;
	return 1;
}

int Archive::append(StringArray& files)
{
	return _m->append(files);
}

int Archive::extract(void* _files)
{
	return _m->extract(_files);
}

int Archive::remove(StringArray& files)
{
	return _m->remove(files);
}

int Archive::rename(StringArray& files)
{
	return _m->rename(files);
}

int Archive::impl::open(const String& name, byte _mode)
{
	mode = 0;

	if(!archive.open(name, 1))
	{
		errorId = 1;
		errorStr = "Can not open archive";
		archive.close();
		return 0;
	}

	// read archive head
	archive.readBuffer(&arcHead, archiveHeadSize);
	archiveSize = fileSize(name);

	// check header crc
	crc.reset();
	crc.calculate((byte*) &arcHead, archiveHeadSize - 4);
	if(arcHead.crcHeader != crc.get())
	{
		errorId = 2;
		errorStr = "Archive header is damaged";
		archive.close();
		return 0;
	}

	// check header signature
	if(arcHead.signature != ARCHIVE_SIGNATURE)
	{
		errorId = 3;
		errorStr = "Archive format not supported";
		archive.close();
		return 0;
	}

	if(!arcHead.filesCount)
	{
		errorId = 5;
		errorStr = "Archive is damaged";
		archive.close();
		return 0;
	}

	// open state
	mode = 2;

	// decode props
	uint props = arcHead.encodeProps;
	encodeMethod = props & 0x0F;
	if(encodeMethod and encodeMethod < 3)
	{
		encodeLevel = (props >> 28) & 0x0F;
		dictSize = 1 << ((props >> 23) & 0x1F);
		if(encodeMethod == 1)
		{
			matchMax = 1 << ((props >> 19) & 0x0F);
			matchMin = (props >> 15) & 0x0F;
			matchRolz = (props >> 13) & 0x03;
			encodeThreads = (props >> 11) & 0x03;
			encodeCmix = (props >> 10) & 0x01;
		}
		if(encodeMethod == 2)
			matchMax = (props >> 19) & 0x0F;
		encodeBigraph = (props >> 4) & 0x01;
	}

	// save all params
	uint* param = &encodeMethod;
	forn(11) paramsOpen[i] = param[i];

	// crypt state
	cryptState = arcHead.options & 0x03;

	// skip build files list, because skip check password
	if(cryptState & 1 and _mode == 2)
		return 1;

	if((_mode == 1 and cryptState != 0) or cryptState & 1)
	{
		if(cryptLength == 0)
		{
			errorId = 4;
			errorStr = "Crypt string is empty";
			return 0;
		}
		crc.reset();
		crc.calculate((byte*) cryptString.p(), cryptLength);
		if(arcHead.crcCrypt != crc.get())
		{
			errorId = 4;
			errorStr = "Crypt string is wrong";
			return 0;
		}
	}

	uint filesCount = arcHead.filesCount;

	// build files list
	ArchiveFileHeader* fileHeads = new ArchiveFileHeader[filesCount];
	String fileName;
	fileList.reserve(filesCount);

	// read file heads
	archive.readBuffer(fileHeads, filesCount * archiveFileHeadSize);

	wint totalSize = 0;
	if(cryptState & 1)
	{
		forn(filesCount)
		totalSize += fileHeads[i].nameLen;
		if(totalSize < 16)
			totalSize = 16;
	}

	uint cryptPos = 0;
	uint cryptPosw = 0;
	if(cryptState & 1)
	{
		assert(cryptLength);
		encryption.keyExpansion((byte*)cryptString.p(), cryptLength);
	}

	// read file names
	UString& fileNameUni = bufferu;
	forn(filesCount)
	{
		ArchiveFileList* list = new ArchiveFileList;
		fileNameUni.clear();

		if((cryptState & 1) == 0)
		{
			fileNameUni.reserve(fileHeads[i].nameLen + 10);
			archive.readBuffer(fileNameUni.p(), fileHeads[i].nameLen);
			fileNameUni[fileHeads[i].nameLen] = 0;
			fileNameUni.update();
			list->name.assign(fileNameUni.p(), fileHeads[i].nameLen);
		}

		// decrypt name
		if(cryptState & 1)
		{
			fileNameUni.reserve(fileHeads[i].nameLen + 10);
			form(u, fileHeads[i].nameLen)
			{
				if(cryptBufferSize == 0)
				{
					cryptBufferSize = 512;
					if(cryptBufferSize > totalSize)
						cryptBufferSize = totalSize;
					if(totalSize > 512 and totalSize < 528)
						cryptBufferSize = 480;
					totalSize -= cryptBufferSize;
					archive.readBuffer(cryptBuffer, cryptBufferSize);
					encryption.decrypt(cryptBuffer, cryptBufferSize, cryptPos);
					cryptPos += cryptBufferSize;
					cryptPosw = 0;
				}
				fileNameUni[u] = cryptBuffer[cryptPosw++];
				cryptBufferSize--;
			}
			fileNameUni[fileHeads[i].nameLen] = 0;
			fileNameUni.update();
			list->name.assign(fileNameUni.p(), fileHeads[i].nameLen);
		}
		form(u, list->name.length())
		{
			if(list->name[u] == PATH_SEP_REV)
				list->name[u] = PATH_SEP;
		}
		list->size = fileHeads[i].size;
		list->crc = fileHeads[i].crc;
		sourceFileSize += fileHeads[i].size;
		list->pos = 0;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		list->time[0] = fileHeads[i].time[0];
		list->time[1] = fileHeads[i].time[1];
#endif
		fileList.push_back(list);
		totalFileHeadSize += archiveFileHeadSize + fileHeads[i].nameLen;
	}
	totalFileHeadSize += archiveHeadSize;

	// read preprocessed bigraph file size
	if(encodeBigraph)
		bigraphSize = arcHead.procSize;

	// save offset to data
	dataPos = archive.tellw();

	wint pos = encodeMethod ? 0 : archive.tellw();
	forn(fileList.size())
	{
		fileList[i]->pos = pos;
		if(!encodeMethod and cryptState & 2 and fileList[i]->size < 16) // AES 128
			pos += 16;
		else
			pos += fileList[i]->size;
	}

	ratioIn = archiveSize * 100 / sourceFileSize;
	safe_delete_array(fileHeads);
	return 1;
}

int Archive::impl::close(byte smode)
{
	mode = 0;
	errorId = 0;
	archiveSize = 0;
	sourceFileSize = 0;
	totalFileHeadSize = 0;
	deleteState = 0;
	archive.close();
	fileList.clear_ptr();
	safe_delete(log);
	if(pathDelete and decPath.length())
	{
		removeFileShell(decPath);
		decPath.clear();
	}
	if(decFile.length())
	{
		removeFile(decFile);
		decFile.clear();
	}
	if(smode)
		cryptString.clear();
	cryptStringSave.clear();
	return 1;
}

int Archive::impl::checkCrypt()
{
	// compare crc32 current crypt string with crypts from header
	if(!cryptState)
		return 1;
	if(!cryptString.length())
		return 0;
	crc.reset();
	crc.calculate((byte*) cryptString.p(), cryptLength);
	if(arcHead.crcCrypt != crc.get())
		return 0;
	return 1;
}

int Archive::impl::append(StringArray& files)
{
	if(!archive.state())
	{
		errorId = 1;
		errorStr = "Archive was not created";
		return 0;
	}
	if(!files.size())
	{
		errorId = 2;
		errorStr = "No appending files";
		return 0;
	}
	if(!range(encodeMethod, 2, 0))
	{
		errorId = 3;
		errorStr = "Encoding method not in range [0, 2]";
		return 0;
	}

	uint decPathLen = 0;
	uint modeUpdate = 0; // append in open archive
	if(mode == 2 and !deleteState)
		modeUpdate = 1;

	if(log and !deleteState)
	{
		log->setConsoleOutput(1);
		if(modeUpdate)
			logWriteLinef("Append %ls\n", archive.getName().p());
		else
			logWriteLinef("Create %ls\n", archive.getName().p());
		log->setConsoleOutput(0);
	}

	// acrhive opened, append files in exist archive
	if(modeUpdate)
	{
		if(!update(files))
		{
			stdprintf("update fail \n");
			return 0;
		}
		decPathLen = decPath.length() + 1;
		logWriteLinef("Append %ls\n", archive.getName().p());
	}

	encodeLevel = clamp(encodeLevel, 9, 1);
	uint filesCount = files.size();
	byte* filesRep = modeUpdate ? new byte[filesCount] : 0; // update
	ArchiveFileHeader* fileHeads = new ArchiveFileHeader[filesCount];
	timer.reset();

	forn(filesCount)
	{
		if(filesRep) filesRep[i] = 0;
	}

	if(encodeMethod)
	{
		StringArray filesOut;
		sortFileExt(files, filesOut);
		files = filesOut;
	}

	// store files count to archive head
	menset(&arcHead, 0, archiveHeadSize);
	arcHead.signature = ARCHIVE_SIGNATURE;
	arcHead.filesCount = filesCount;
	arcHead.dataSize = 0;
	arcHead.options |= cryptState;
	archive.storeBuffer(&arcHead, archiveHeadSize);
	archiveSize += archiveHeadSize;

	if(cryptState)
		assert(cryptString.length());

	sourceFileSize = 0;

	// store files heads [empty]
	archive.storeBuffer(fileHeads, filesCount * archiveFileHeadSize);

	wint cryptTotal = 0;
	wint cryptPos = 0;
	if(cryptState & 3)
		encryption.keyExpansion((byte*)cryptString.p(), cryptLength);

	// store files list
	totalFileHeadSize = archiveHeadSize;
	forn(filesCount)
	{
		// exist archive
		uint skipFileCur = skipFileLen;
		byte rep = 0; // repository, unpacked
		if(modeUpdate)
		{
			// skip length from extracted file
			if(files[i].compare(decPath, 0, decPathLen - 1) == 0)
			{
				skipFileCur = decPathLen;
				rep = 1;
				filesRep[i] = 1;
			}
		}

		if(!rep and !skipFileCur and files[i][1] == ':')
			skipFileCur = files[i].findr(PATH_SEP) + 1;

		if(pathArchive.length() and !rep)
			buffer.format("%ls%c%ls", pathArchive.p(), PATH_SEP, files[i].p() + skipFileCur);
		else
			buffer.assign(files[i], skipFileCur);

		form(j, buffer.length())
		{
			if(buffer[j] == PATH_SEP_REV)
				buffer[j] = PATH_SEP;
		}
		int sp = buffer.findr(PATH_SEP_UP); // ..\e.
		if(sp != -1)
			buffer.substrs(sp + 3);
		getUnicode(buffer);
		fileHeads[i].nameLen = bufferu.length();
		fileHeads[i].size = fileSize(files[i]);
		archiveSize += archiveFileHeadSize + fileHeads[i].size + fileHeads[i].nameLen;
		sourceFileSize += fileHeads[i].size;
		totalFileHeadSize += archiveFileHeadSize + fileHeads[i].nameLen;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		fileTimeValue(files[i], fileHeads[i].time, 2);
#endif

		// crypt file name
		if(cryptState & 1)
		{
			form(u, fileHeads[i].nameLen)
			{
				cryptBuffer[cryptBufferSize++] = bufferu[u];
				cryptTotal++;
				if(cryptBufferSize == 512 or (u == fileHeads[i].nameLen-1 and i == filesCount-1))
				{
					if(cryptBufferSize < 16)
					{
						if(cryptTotal < 16)
						{
							// padding with zeros
							form(q, 16 - cryptBufferSize)
							cryptBuffer[q + cryptBufferSize] = 0;
							cryptBufferSize = 16;
						}
						else
						{
							// stealing from penultimate block to first block
							// copy penultimate block size to next block
							form(q, 16 - cryptBufferSize)
							cryptBuffer[q + cryptBufferSize] = cryptBuffer[q + cryptBufferSize + 496];
							form(q, cryptBufferSize)
							cryptBuffer[q + 16] = cryptBuffer[q + 496];
							archive.seekw(-16, 1);
							encryption.encrypt(cryptBuffer, 16, cryptPos);
							archive.storeBuffer(cryptBuffer, cryptBufferSize + 16);
							cryptBufferSize = 0;
						}
					}
					// crypt, save
					if(cryptBufferSize)
					{
						encryption.encrypt(cryptBuffer, cryptBufferSize, cryptPos);
						archive.storeBuffer(cryptBuffer, cryptBufferSize);
					}
					cryptPos += cryptBufferSize;
					cryptBufferSize = 0;
				}
			}
		}
		else
			archive.storeBuffer(bufferu.p(), fileHeads[i].nameLen);
	}

	uint memSize = BLOCK_SIZE;
	if(!mem) mem = new byte[memSize];
	uint readBytes = 0;
	wint curSize = 0;
	wint totSize = 0;
	uint curNum = 0;
	uint progress = 0;
	FileStream appendFile;
	crc.reset();

	uint memEncSize = BLOCK_SIZE;
	SmartPtr<ICoder> encoder;
	SmartPtr<BigraphCoder> encoderBigraph;
	CoderStream coderStream;

	if(encodeMethod)
	{
		wint sourceFileSizeEncode = sourceFileSize + 4096;
		if(sourceFileSizeEncode < dictSize)
			dictSize = sourceFileSizeEncode;
		else if(dictSize == 0)
			dictSize = MIN(sourceFileSizeEncode, 1 << 24);

		if(!memEnc) memEnc = new byte[memEncSize];

		coderStream.src = mem;
		coderStream.dst = memEnc;
		coderStream.srcAvail = memSize;
		coderStream.dstAvail = memEncSize;

		if(encodeBigraph)
		{
			encoderBigraph.set(new BigraphCoder);
			encoderBigraph->initialise(1);
		}

		encoder.set(createMethod(0));

		// cm decoder need source file size
		if(encodeMethod == 2)
			arcHead.dataSize = 8;
		if(encodeMethod == 2 and encodeBigraph == 0)
		{
			if(cryptState & 2)
			{
				mencpy(cryptBuffer, &sourceFileSize, 8);
				cryptBufferSize += 8;
			}
			else
				archive.storeWint(sourceFileSize);
		}
	}

	byte exits = 0;
	wint eventData[10] = {0};
	uint timeCur[3] = {0}; // minutes, seconds, mseconds;

	archiveHeadSizeTotal = archive.tellw();

	/*
		0: preprocessing parsing
		1: preprocessing encode
		2: compression
		N: encryption
	*/

	byte mainLoop = encodeBigraph ? 3 : 1;
	String bigraphFile;
	FileStream appendFileBigraph;
	wint sourceSize = sourceFileSize;
	cryptPos = 0;
	cryptBufferSize = 0;

	form(j, mainLoop)
	{
		byte crccalc = 0;
		if(mainLoop == 1 or j == 1 or (j == 2 and !encodeBigraph))
			crccalc = 1;

		memSize = BLOCK_SIZE;
		curNum = 1;
		totSize = 0;
		uint filesn = files.size();
		// 2: compression
		if(j == 2 and encodeBigraph)
			filesn = 1;

		// bigraph encode
		if(j == 1)
		{
			bigraphFile.reserve(1024);
			buffer = "dla.bg.";
			generateFileName(bigraphFile, buffer, 0, 6);
			bigraphFile.update();
			int state = appendFileBigraph.open(bigraphFile, 0);
			assert(state);
			encoderBigraph->initialise(2);
		}

		// store files to archive
		forn(filesn)
		{
			byte evtUpd = 1;
			memSize = BLOCK_SIZE;
			bool state = 0;
			// step 1: encode from bigraph parsing
			// step 2: compress from bigraph data
			if(j == 2 and encodeBigraph)
			{
				buffer = bigraphFile;
				curSize = fileSize(bigraphFile.p());
				sourceSize = curSize;
				// to archive bigraph file size after crc
				arcHead.procSize = sourceSize;
			}
			else
			{
				buffer = files[i];
				curSize = fileHeads[i].size;
			}

			/* cm decoder need source file size */
			if(j == 2 and encodeMethod == 2)
			{
				if(cryptState & 2)
				{
					mencpy(cryptBuffer, &sourceSize, 8);
					cryptBufferSize += 8;
				}
				else
					archive.storeWint(sourceSize);
			}

			if(!appendFile.open(buffer, 1))
			{
				errorId = 4;
				errorStr.format("Can not open file %ls", buffer.p());
				return 0;
			}

			crc.reset();

			if(!encodeMethod)
				cryptPos = 0;

			while(curSize)
			{
				if(encodeMethod == 0 and cryptState & 2 and
				        curSize > memSize and curSize < memSize + 16)
					memSize -= 128;

				readBytes = appendFile.readBuffer(mem, memSize);

				assert(readBytes);
				curSize -= readBytes;
				totSize += readBytes;

				// encode after bigraph
				if(j == 2 and encodeBigraph)
				{
					curNum = clamp(totSize / (sourceSize / filesCount), filesCount, 1);
				}

				// crc source files
				if(crccalc)
					crc.calculate(mem, readBytes);

				// crypt stream
				if(encodeMethod == 0 and cryptState & 2)
				{
					// padding with zeros
					if(readBytes < 16)
					{
						form(u, 16 - readBytes)
						mem[u + readBytes] = 0;
						readBytes = 16;
					}
					encryption.encrypt(mem, readBytes, cryptPos);
					cryptPos += readBytes;
				}

				if(encodeMethod)
				{
					byte end = curSize == 0 and i == filesn - 1;
					coderStream.srcAvail = readBytes;

					int ret = IS_STREAM_END;
					while(ret == IS_STREAM_END)
					{
						coderStream.dstAvail = memEncSize;
						if(encodeBigraph and j == 0)
							ret = encoderBigraph->parsing(&coderStream, end);
						else if(j == 1)
						{
							ret = encoderBigraph->compress(&coderStream, end);
							appendFileBigraph.storeBuffer(coderStream.dst, coderStream.dstAvail);
						}
						else
						{
							ret = encoder->compress(&coderStream, end);
							assert(coderStream.dstAvail != 0);

							// crypt stream
							if(cryptState & 2)
							{
								form(u, coderStream.dstAvail)
								{
									cryptBuffer[cryptBufferSize++] = coderStream.dst[u];
									if(cryptBufferSize == 512 or (u == coderStream.dstAvail-1 and end))
									{
										if(cryptBufferSize < 16) // coverage complete [00]
										{
											if(coderStream.dstTotal < 16)
											{
												// padding with zeros
												arcHead.dataSize += 16 - cryptBufferSize;
												form(q, 16 - cryptBufferSize)
												cryptBuffer[q + cryptBufferSize] = 0;
												cryptBufferSize = 16;
											}
											else
											{
												// stealing from penultimate block to first block
												// copy penultimate block size to next block
												form(q, 16 - cryptBufferSize)
												cryptBuffer[q + cryptBufferSize] = cryptBuffer[q + cryptBufferSize + 496];
												form(q, cryptBufferSize)
												cryptBuffer[q + 16] = cryptBuffer[q + 496];
												archive.seekw(-16, 1);
												encryption.encrypt(cryptBuffer, 16, cryptPos);
												archive.storeBuffer(cryptBuffer, cryptBufferSize + 16);
												cryptBufferSize = 0;
											}
										}
										// crypt, save
										if(cryptBufferSize)
										{
											encryption.encrypt(cryptBuffer, cryptBufferSize, cryptPos);
											archive.storeBuffer(cryptBuffer, cryptBufferSize);
										}
										cryptPos += cryptBufferSize;
										cryptBufferSize = 0;
									}
								}
							}
							else
								archive.storeBuffer(coderStream.dst, coderStream.dstAvail);
						}
					}
					if(ret != IS_OK)
					{
						if(encodeBigraph and j == 0)
						{
							_printf("\nstop bigraph\n");
							encodeBigraph = 0;
							j++;
							break;
						}
						else
						{
							errorId = 5;
							errorStr = "Compression fail";
							return 0;
						}
					}
				}
				else
					archive.storeBuffer(mem, readBytes);

				if(log)
				{
					getTime();
					progress = totSize * 100 / sourceSize;
					stdprintf("\rProgress %3u | %u / %u | %02u:%02u:%03u",
					          progress, curNum, filesCount, minutes, seconds, msecondsp);
				}

				if(event_callback)
				{
					/* [0] total processed size
						[1] total need process size [maximum]
						[2] compression ratio
						[3] current count processed files
						[4] total files count
						[5] current file name
						[6] time pointer to array of 3 elements: minutes seconds mseconds
					*/
					eventData[0] = totSize;
					eventData[1] = sourceSize;
					eventData[2] = coderStream.dstTotal * 100 / totSize;
					eventData[3] = curNum;
					eventData[4] = filesCount;
					eventData[5] = 0;
					if(evtUpd == 1)
					{
						uint in = curNum - 1;
						eventData[5] = (wint) &files[in];
						evtUpd = 0;
					}
					timeCur[0] = minutes;
					timeCur[1] = seconds;
					timeCur[2] = mseconds;
					eventData[6] = (wint) timeCur;
					if(!event_callback(eventData))
					{
						exits = 1;
						break;
					}
				}
			}

			appendFile.close();
			if(crccalc)
				fileHeads[i].crc = crc.get();

			if(exits)
			{
				if(log)
					log->writeLine("\nAppending stoped");
				safe_delete_array(fileHeads);
				safe_delete_array(filesRep);
				if(bigraphFile.length())
				{
					appendFileBigraph.close();
					removeFile(bigraphFile);
				}
				return 0;
			}

			if(log and crccalc)
			{
				uint skip = skipFileLen;
				if(modeUpdate and filesRep[i])
					skip = decPathLen;
				else if(!skip and files[i][1] == ':')
					skip = files[i].findr(PATH_SEP) + 1;
				if(pathArchive.length() and !filesRep[i])
					buffer.format("%ls\\%ls", pathArchive.p(), files[i].p() + skip);
				else
					buffer.assign(files[i], skip);
				log->writeLine(getUnicode(buffer));
			}
			curNum++;
		}
		if(j == 1)
			appendFileBigraph.close();
	}

	archiveSize = archive.tellw();
	if(encodeMethod)
		arcHead.dataSize += coderStream.dstTotal;
	else
		arcHead.dataSize = sourceFileSize;

	if(log)
		logTime(filesCount);

	// props size 32 bit
	// dict size 5 bit 2 ^ n
	// word size 4 bit 2 ^ n
	// encode level 4 bit
	// encode threads 4 bit
	// encode minutes match len 4 bit
	// encode bigraph 1 bit
	// encode method 4 bit [low bits]
	// free 6 bit

	// Encoding properties from most significant bits.
	// Free 6 bits.
	// Dictonary, word is power of 2
	// Rolz range [0, 3] -> [16, 64, 128, 256]
	// --------------------------------------------
	// | i | description  | bit | shift | methods |
	// |------------------------------------------|
	// | 0 | level        |  4  |  28   | all     |
	// | 1 | dictionary   |  5  |  23   | all     |
	// | 2 | word         |  4  |  19   | all     |
	// | 3 | match minutes    |  4  |  15   | lzre    |
	// | 4 | rolz         |  2  |  13   | lzre    |
	// | 5 | threads      |  2  |  11   | lzre    |
	// | 6 | cmix         |  1  |  10   | lzre    |
	// | 7 | bigraph      |  1  |   4   | all     |
	// | 8 | method       |  4  |   0   | all     |
	// --------------------------------------------

	if(encodeMethod)
	{
		uint props = encodeMethod;

		if(encodeMethod < 3)
		{
			props |= encodeLevel << 28;
			props |= bitGreat(dictSize - 1) << 23;

			if(encodeMethod == 1)
			{
				props |= bitGreat(matchMax - 1) << 19;
				props |= matchMin << 15;
				props |= matchRolz << 13;
				props |= encodeThreads << 11;
				props |= encodeCmix << 10;
			}
			if(encodeMethod == 2)
				props |= matchMax << 19;
			if(encodeBigraph)
				props |= 1 << 4;
		}
		arcHead.encodeProps = props;
	}

	// reset values
	uint* param = &encodeMethod;
	forn(9) param[i] = 0;

	crc.reset();
	crc.calculate((byte*) cryptString.p(), cryptLength);
	arcHead.crcCrypt = crc.get();

	// get archive header crc
	crc.reset();
	crc.calculate((byte*) &arcHead, archiveHeadSize - 4);
	arcHead.crcHeader = crc.get();

	// store archive header
	archive.seekw(0, 0);
	archive.storeBuffer(&arcHead, archiveHeadSize);

	// store files headers
	archive.storeBuffer(fileHeads, filesCount * archiveFileHeadSize);

	safe_delete_array(filesRep);
	safe_delete_array(fileHeads);
	if(bigraphFile.length())
		removeFile(bigraphFile);

	if(mode == 2)
		removeFileShell(decPath);

	safe_delete(log);
	return 1;
}

int Archive::impl::extract(void* _files)
{
	errorId = 0;
	if(!archive.state())
	{
		errorId = 1;
		errorStr = "Archive was not open";
		return 0;
	}

	if(!checkCrypt())
	{
		errorId = 6;
		errorStr = "Crypt string is wrong";
		return 0;
	}

	uint* filesn = 0;
	uint filesnn = 0;
	wint cumFileSize = 0;
	uint emptyList = 0;
	byte srch = 0;
	crc.reset();
	timer.reset();

	if(_files)
	{
		if(!extractMode)
		{
			StringArray* files = (StringArray*) _files;
			if(files->size())
			{
				srch = 1;
				filesn = new uint[fileList.size()];
				forn(fileList.size())
				{
					form(u, files->size())
					{
						if(stwpat(fileList[i]->name.p(), (*files)[u].p()))
						{
							filesn[filesnn++] = i;
							cumFileSize += fileList[i]->size;
							break;
						}
					}
				}
			}
			else
				emptyList = 1;
		}
		else // one file
		{
			srch = 1;
			filesn = new uint;
			forn(fileList.size())
			{
				if(stwpat(fileList[i]->name.p(), (wchar_t*) _files))
				{
					filesn[filesnn++] = i;
					cumFileSize += fileList[i]->size;
					break;
				}
			}
		}
	}

	if(srch and !filesnn)
	{
		errorId = 2;
		errorStr = "Extracting files not found";
		safe_delete_array(filesn);
		return 0;
	}

	if(!_files or emptyList)
		cumFileSize = sourceFileSize;

	if(log and !deleteState)
	{
		log->setConsoleOutput(1);
		logWriteLinef("Unpack %ls\n", archive.getName().p());
		log->setConsoleOutput(0);
	}

	uint filesCount = filesnn ? filesnn : fileList.size();
	uint fileIn = 0;
	String fileName;
	String filePath;
	String filePathRd;
	wint fileSize = 0;
	wint filePos = 0;

	uint memSize = BLOCK_SIZE;
	uint readSize = memSize;
	if(!mem) mem = new byte[memSize];
	uint readBytes = 0;
	wint totSize = 0;
	uint progress = 0;
	uint curNum = 1;
	byte crcFail = 0;
	FileStream extractFile;

	FileStream decodedFile;
	uint memEncSize = BLOCK_SIZE; // uncompressed block size
	SmartPtr<ICoder> decoder;
	CoderStream coderStream;

	SmartPtr<BigraphCoder> decoderBigraph;
	FileStream decodedFilePrc;

	if(encodeMethod and !decFile.length())
	{
		wint sourceFileSizeDecode = sourceFileSize + 4096;
		if(sourceFileSizeDecode < dictSize)
			dictSize = sourceFileSizeDecode;

		if(!memEnc) memEnc = new byte[memEncSize];

		coderStream.src = mem;
		coderStream.dst = memEnc;
		coderStream.srcAvail = memSize;
		coderStream.dstAvail = memEncSize;

		if(encodeBigraph)
		{
			decoderBigraph.set(new BigraphCoder);
			decoderBigraph->initialise(3);
		}

		decoder.set(createMethod(1));
	}

	byte exits = 0;
	wint eventData[10] = {0};
	uint timeCur[3] = {0}; // minutes, seconds, mseconds
	String uncFile("uncompressed file");
	uint cryptPos = 0;
	if(cryptState & 3)
		encryption.keyExpansion((byte*)cryptString.p(), cryptLength);

	if(encodeMethod and decFile.length())
	{
		if(!decodedFile.open(decFile, 0, "rb"))
			return 0;
	}

	// extract / decompress files to temp
	if(encodeMethod and !decFile.length())
	{
		byte evtUpd = 1;
		byte mainLoop = 1;
		archive.seekw(dataPos, 0);

		if(encodeBigraph)
			mainLoop = 2;

		filePath.reserve(1024);

		wint sourceSize = sourceFileSize;
		if(encodeBigraph)
			sourceSize = bigraphSize;

		form(j, mainLoop)
		{
			readSize = memSize;
			totSize = 0;
			// decode file data
			fileSize = arcHead.dataSize;
			if(j == 1)
			{
				sourceSize = sourceFileSize;
				fileSize = tas::fileSize(filePath);
				filePathRd = filePath;
				decodedFilePrc.open(filePath, 1);
			}

			if(!j and archiveSize - archive.tellw() != fileSize)
			{
				errorId = 3;
				errorStr = "Compressed stream is damaged";
				return 0;
			}
			buffer.format("dla.%c.", 'a' + j);
			generateFileName(filePath, buffer, 0, 6);
			filePath.update();
			if(mainLoop == 1 or j == 1)
				decFile = filePath;

			// open temp file for read/write "wb+"
			if(!decodedFile.open(filePath, 0, "wb+"))
				return 0;

			while(fileSize)
			{
				if(fileSize < readSize)
					readSize = fileSize;

				if(cryptState & 2 and !j and
				        fileSize > memSize and fileSize < memSize + 16)
				{
					readSize -= 128;
				}

				if(!j)
					readBytes = archive.readBuffer(mem, readSize);
				else
					readBytes = decodedFilePrc.readBuffer(mem, readSize);

				assert(readBytes);
				totSize += readBytes;
				fileSize -= readBytes;
				coderStream.srcAvail = readBytes;

				// decrypt stream
				if(!j and cryptState & 2)
				{
					encryption.decrypt(mem, readBytes, cryptPos);
					cryptPos += readBytes;
				}

				int ret = IS_STREAM_END;
				while(ret == IS_STREAM_END)
				{
					coderStream.dstAvail = memEncSize;
					if(!j)
						ret = decoder->uncompress(&coderStream, fileSize == 0);
					else
						ret = decoderBigraph->uncompress(&coderStream, fileSize == 0);
					assert(coderStream.dstAvail != 0);
					decodedFile.storeBuffer(coderStream.dst, coderStream.dstAvail);
					if(log)
					{
						progress = coderStream.dstTotal * 100 / sourceSize;
						getTime();
						if(!j) stdprintf("\rProgress %3u | %02u:%02u:%03u", progress, minutes, seconds, msecondsp);
					}
					if(!j and event_callback and log)
					{
						ratioUn = coderStream.srcTotal * 100 / coderStream.dstTotal;
						eventData[0] = coderStream.dstTotal;
						eventData[1] = sourceSize;
						eventData[2] = ratioUn;
						eventData[3] = 1;
						eventData[4] = 1;
						eventData[5] = 0; // filePath
						if(evtUpd == 1)
						{
							eventData[5] = (wint) &uncFile;
							evtUpd = 0;
						}
						timeCur[0] = minutes;
						timeCur[1] = seconds;
						timeCur[2] = mseconds;
						eventData[6] = (wint) timeCur;
						if(!event_callback(eventData))
						{
							exits = 1;
							break;
						}
					}
					else if(event_callback)
						event_callback(0);
				}
				if(exits)
					break;
				if(ret != IS_OK)
				{
					errorId = 4;
					errorStr.format("Decompression fail %d", ret);
					exits = 1;
					break;
				}
			}
			if(mainLoop == 1 or j)
				decodedFile.seekw(0, 0);
			else
				decodedFile.close();
			if(j)
			{
				decodedFilePrc.close();
				removeFile(filePathRd);
			}
			if(exits)
			{
				decodedFile.close();
				break;
			}
		}
	}

	if(exits)
	{
		_printf("\nExtracting stoped\n\n");
		if(log)
			log->writeLine("\nExtracting stoped");
		safe_delete_array(filesn);
		return 0;
	}

	totSize = 0;

	// extract files
	forn(filesCount)
	{
		fileIn = filesnn ? filesn[i] : i;
		fileSize = fileList[fileIn]->size;
		filePos = fileList[fileIn]->pos;
		readSize = memSize;

		if(savePath)
			fileName.assign(fileList[fileIn]->name, skipFileLen);
		else
			fileName = fileList[fileIn]->name.substrx(PATH_SEP, 1, 1);

		if(decPath.length())
			filePath.format("%ls%c%ls", decPath.p(), PATH_SEP, fileName.p());
		else
			filePath = fileName;

		createFolder(filePath);

		if(!extractFile.open(filePath, 0))
			return 0;

		lastFile = filePath;

		if(encodeMethod)
			decodedFile.seekw(filePos, 0);
		else
			archive.seekw(filePos, 0);

		crc.reset();
		byte evtUpd = 1;

		if(!encodeMethod)
		{
			cryptPos = 0;
			if(cryptState & 2 and fileSize < 16)
				fileSize = 16;
		}
		while(fileSize)
		{
			if(fileSize < readSize)
				readSize = fileSize;

			if(encodeMethod == 0 and cryptState & 2 and
			        fileSize > memSize and fileSize < memSize + 16)
			{
				readSize -= 128;
			}

			if(encodeMethod)
				readBytes = decodedFile.readBuffer(mem, readSize);
			else
				readBytes = archive.readBuffer(mem, readSize);

			assert(readBytes);
			fileSize -= readBytes;
			totSize += readBytes;

			// decrypt stream
			if(encodeMethod == 0 and cryptState & 2)
			{
				encryption.decrypt(mem, readBytes, cryptPos);
				cryptPos += readBytes;
			}

			if(!encodeMethod and cryptState & 2 and fileList[fileIn]->size < 16)
				readBytes = fileList[fileIn]->size;

			crc.calculate(mem, readBytes);

			if(log)
			{
				getTime();
				progress = totSize * 100 / cumFileSize;
				stdprintf("\rProgress %3u | %u / %u | %02u:%02u:%03u",
				          progress, curNum, filesCount, minutes, seconds, msecondsp);
			}
			if(log and event_callback)
			{
				eventData[0] = totSize;
				eventData[1] = cumFileSize;
				eventData[2] = ratioUn;
				eventData[3] = curNum;
				eventData[4] = filesCount;
				eventData[5] = 0;
				if(evtUpd == 1)
				{
					eventData[5] = (wint) &filePath;
					evtUpd = 0;
				}
				timeCur[0] = minutes;
				timeCur[1] = seconds;
				timeCur[2] = mseconds;
				eventData[6] = (wint) timeCur;
				if(!event_callback(eventData))
				{
					exits = 1;
					break;
				}
			}
			else if(event_callback)
				event_callback(0);
			extractFile.storeBuffer(mem, readBytes);
		}
		extractFile.close();

		if(exits)
			break;

		if(fileList[fileIn]->crc != crc.get())
			crcFail = 1;

		if(log)
		{
			if(fileList[fileIn]->crc != crc.get())
				logWriteLinef("%ls damaged", fileList[fileIn]->name.p());
			else
				log->writeLine(getUnicode(fileList[fileIn]->name));
		}
		curNum++;
	}

	if(exits)
	{
		if(log)
			log->writeLine("\nExtracting stoped");
		safe_delete_array(filesn);
		return 0;
	}

	if(encodeMethod)
		decodedFile.close();

	if(log)
		logTime(filesCount);

	safe_delete_array(filesn);
	if(crcFail)
	{
		errorId = 5;
		errorStr = "Archive files damaged";
	}

	if(!deleteState)
		safe_delete(log);
	return crcFail == 0;
}

int Archive::impl::update(StringArray& files)
{
	/*
		- extract archive files to temp
		- copy new files to temp
		- close, create, append ...
	*/

	// temp directory
	decPath.reserve(1024);
	buffer = "dla.";
	generateFileName(decPath, buffer, 1, 6);
	decPath.update();
	uint decPathLen = decPath.length() + 1;
	createFolder(decPath, 1);

	// save all params current
	uint* param = &encodeMethod;
	forn(11) paramsCur[i] = param[i];

	// restore params from open
	forn(11) param[i] = paramsOpen[i];

	// extract archive files to temp
	uint skipFileCopy = skipFileLen;

	skipFileLen = 0;
	savePath = 1;
	deleteState = 1;

	// save crypt
	byte cryptStateBackup = cryptState;
	cryptState = arcHead.options & 0x03;

	// save pointer current password
	// restore saved password
	UString cryptStringBackup;
	if(cryptStringSave.length())
	{
		cryptStringBackup = cryptString;
		cryptString = cryptStringSave;
		cryptLength = cryptString.length();
		cryptString.resize(cryptLength + 32); // fill nulls
		cryptString.resize(cryptLength); // restore size
		if(cryptLength % 16)
			cryptLength += 16 - (cryptLength % 16);
	}

	log->setConsoleOutput(0);
	int ret = extract(0);
	cryptState = cryptStateBackup;

	// restore current password
	if(cryptStringSave.length())
	{
		cryptString = cryptStringBackup;
		cryptLength = cryptString.length();
		cryptString.resize(cryptLength + 32); // fill nulls
		cryptString.resize(cryptLength); // restore size
		cryptStringSave.clear();
		if(cryptLength % 16)
			cryptLength += 16 - (cryptLength % 16);
	}

	deleteState = 0;
	skipFileLen = skipFileCopy;
	if(!ret)
	{
		_printf("extract fail \n");
		return 0;
	}
	log->reopen(0);

	// restore from current
	forn(11)
	param[i] = paramsCur[i];

	// update files list, append unique extracted files
	uint filesn = files.size();
	forn(fileList.size())
	{
		// exclude equal files
		// not append in list
		byte skip = 0;
		form(u, filesn)
		{
			skipFileLen = skipFileCopy;
			if(!skipFileLen and files[u][1] == ':')
				skipFileLen = files[u].findr(PATH_SEP) + 1;
			if(pathArchive.length())
				buffer.format("%ls\\%ls", pathArchive.p(), files[u].p() + skipFileLen);
			else
				buffer.assign(files[u], skipFileLen);
			if(fileList[i]->name == buffer)
			{
				skip = 1;
				break;
			}
		}
		skipFileLen = skipFileCopy;
		if(skip)	continue;
		buffer.format("%ls\\%ls", decPath.p(), fileList[i]->name.p());
		files.push_back(buffer);
	}

	// close, create
	// before close save crypt string
	buffer = archive.getName();
	archive.close();
	archive.open(buffer, 0);

	return 1;
}

int Archive::impl::remove(StringArray& files)
{
	if(!archive.state())
	{
		errorId = 1;
		errorStr = "Archive was not created";
		return 0;
	}

	if(!files.size())
	{
		errorId = 2;
		errorStr = "No deleting files";
		return 0;
	}

	/*
		- build list unpack
		- generate unpack path
		- unpack
		- build list append
		- append
	*/

	StringArray listUnpack;
	StringArray listAppend;
	listUnpack.reserve(fileList.size());

	forn(fileList.size())
	{
		bool find = 0;
		form(u, files.size())
		{
			if(stwpat(fileList[i]->name.p(), files[u].p()))
			{
				find = 1; // deleted file
				break;
			}
		}
		if(!find)
			listUnpack.push_back(fileList[i]->name);
	}

	if(listUnpack.size() == fileList.size())
	{
		errorId = 3;
		errorStr = "No deleted files";
		return 0;
	}

	if(listUnpack.size() == 0)
	{
		errorId = 4;
		errorStr = "Deleting all files";
		return 0;
	}

	log->setConsoleOutput(1);
	logWriteLinef("Delete %ls\n", archive.getName().p());
	log->setConsoleOutput(0);

	decPath.reserve(1024);
	buffer = "dla.";
	generateFileName(decPath, buffer, 1, 6);
	decPath.update();
	createFolder(decPath, 1);
	deleteState = 1;
	skipFileLen = 0;
	savePath = 1;
	int ret = extract(&listUnpack);
	if(!ret)
	{
		_printf("extract fail \n");
		return 0;
	}
	log->reopen(0);
	logWriteLinef("Delete %ls\n", archive.getName().p());

	listAppend.reserve(listUnpack.size());
	forn(listUnpack.size())
	{
		buffer.format("%ls%c%ls", decPath.p(), PATH_SEP, listUnpack[i].p());
		listAppend.push_back(buffer);
	}

	String rname = archive.getName();
	archive.close();
	archive.open(rname, 0);
	skipFileLen = decPath.length() + 1;

	ret = append(listAppend);

	if(!ret)
	{
		_printf("append fail \n");
		return 0;
	}

	// reopen archive
	close();

	ret = open(rname);
	if(!ret)
	{
		assert(0);
		return 0;
	}

	deleteState = 0;
	safe_delete(log);
	return 1;
}

int Archive::impl::rename(StringArray& files)
{
	if(!archive.state())
	{
		errorId = 1;
		errorStr = "Archive was not created";
		return 0;
	}

	uint renameDirectory = 0;
	uint filesCountIn = files.size();
	uint filesCountDiv = filesCountIn / 2;

	if(filesCountIn < 2)
	{
		errorId = 2;
		errorStr = "No renaming files";
		return 0;
	}

	uint length = files[0].length();
	if(files[0][length - 1] == PATH_SEP)
	{
		renameDirectory = 1;
		if(filesCountIn != 2)
		{
			errorId = 3;
			errorStr = "Can rename single folder";
			return 0;
		}
	}
	else
	{
		if(filesCountIn % 2)
		{
			errorId = 3;
			errorStr = "Count files must be even";
			return 0;
		}
	}

	// search one item
	byte found = 0;
	forn(fileList.size())
	{
		form(u, filesCountDiv)
		{
			if(renameDirectory)
				buffer.format("%ls*", files[u * 2].p());
			else
				buffer = files[u * 2];
			if(stwpat(fileList[i]->name.p(), buffer.p()))
			{
				found = 1;
				break;
			}
		}
		if(found)
			break;
	}

	if(!found)
	{
		errorId = 4;
		errorStr = "Not found files for renaming";
		return 0;
	}

	if(log)
	{
		log->setConsoleOutput(1);
		logWriteLinef("Rename %ls\n", archive.getName().p());
		log->setConsoleOutput(0);
	}

	// generate temp archive name near source
	String archiveName = archive.getName();
	String archiveNameTemp;
	archiveNameTemp.reserve(archiveName.length() + 64);
	buffer.format("%ls.", archive.getName().p());
	generateFileName(archiveNameTemp, buffer, 0, 6, 0);

	// destination temp archive
	FileStream archiveDest(archiveNameTemp, 0);
	if(!archiveDest.state())
		return 0;

	// archive header from source archive to dest
	archiveDest.storeBuffer(&arcHead, archiveHeadSize);

	// file list
	uint filesCount = fileList.size();

	ArchiveFileHeader* fileHeads = new ArchiveFileHeader[filesCount];

	// store files headers [empty]
	archiveDest.storeBuffer(fileHeads, filesCount * archiveFileHeadSize);

	String curFile;
	UString curFileUni;
	wint cryptTotal = 0;
	wint cryptPos = 0;
	if(cryptState & 3)
		encryption.keyExpansion((byte*)cryptString.p(), cryptLength);

	forn(filesCount)
	{
		found = 0;
		fileHeads[i].size = fileList[i]->size;
		fileHeads[i].crc = fileList[i]->crc;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		fileHeads[i].time[0] = fileList[i]->time[0];
		fileHeads[i].time[1] = fileList[i]->time[1];
#endif
		form(u, filesCountDiv)
		{
			if(renameDirectory)
				buffer.format("%ls*", files[u * 2].p());
			else
				buffer = files[u * 2];
			if(stwpat(fileList[i]->name.p(), buffer.p()))
			{
				if(renameDirectory)
					curFile.format("%ls%ls", files[u * 2 + 1].p(), fileList[i]->name.p() + files[u * 2].length() - 1);
				else
					curFile = files[u * 2 + 1];
				if(log)
					logWriteLinef("%ls [rename]", curFile.p());
				curFileUni.assign(curFile.p(), curFile.length());
				fileHeads[i].nameLen = curFileUni.length();
				found = 1;
				break;
			}
		}
		if(!found)
		{
			curFileUni.assign(fileList[i]->name.p(), fileList[i]->name.length());
			fileHeads[i].nameLen = curFileUni.length();
			if(log)
				log->writeLine(curFileUni.p());
		}

		// crypt
		if(cryptState & 1)
		{
			form(u, fileHeads[i].nameLen)
			{
				cryptBuffer[cryptBufferSize++] = curFileUni[u];
				cryptTotal++;
				if(cryptBufferSize == 512 or (u == fileHeads[i].nameLen-1 and i == filesCount-1))
				{
					if(cryptBufferSize < 16)
					{
						if(cryptTotal < 16)
						{
							// padding with zeros
							form(q, 16 - cryptBufferSize)
							cryptBuffer[q + cryptBufferSize] = 0;
							cryptBufferSize = 16;
						}
						else
						{
							// stealing from penultimate block to first block
							// copy penultimate block size to next block
							form(q, 16 - cryptBufferSize)
							cryptBuffer[q + cryptBufferSize] = cryptBuffer[q + cryptBufferSize + 496];
							form(q, cryptBufferSize)
							cryptBuffer[q + 16] = cryptBuffer[q + 496];
							archiveDest.seekw(-16, 1);
							encryption.encrypt(cryptBuffer, 16, cryptPos);
							archiveDest.storeBuffer(cryptBuffer, cryptBufferSize + 16);
							cryptBufferSize = 0;
						}
					}
					// crypt, save
					if(cryptBufferSize)
					{
						encryption.encrypt(cryptBuffer, cryptBufferSize, cryptPos);
						archiveDest.storeBuffer(cryptBuffer, cryptBufferSize);
					}
					cryptPos += cryptBufferSize;
					cryptBufferSize = 0;
				}
			}
		}
		else
			// store file name
			archiveDest.storeBuffer(curFileUni.p(), fileHeads[i].nameLen);
	}

	// store files headers
	archiveDest.seekw(archiveHeadSize, 0);
	archiveDest.storeBuffer(fileHeads, filesCount * archiveFileHeadSize);
	archiveDest.seekw(0, 2);

	// store files datas
	archive.seekw(dataPos, 0);

	uint memSize = BLOCK_SIZE;
	if(!mem) mem = new byte[memSize];
	wint curSize = arcHead.dataSize;
	wint sourceSize = curSize;
	wint totSize = 0;
	uint progress = 0;
	uint readBytes = 0;
	timer.reset();

	while(curSize)
	{
		readBytes = archive.readBuffer(mem, memSize);
		assert(readBytes);
		curSize -= readBytes;
		totSize += readBytes;
		archiveDest.storeBuffer(mem, readBytes);
		if(log)
		{
			progress = totSize * 100 / sourceSize;
			getTime();
			stdprintf("\rProgress %3u | %02u:%02u:%03u", progress, minutes, seconds, msecondsp);
		}
	}

	if(log)
		logTime(filesCount);

	// close archives
	archive.close();
	archiveDest.close();
	fileList.clear_ptr();
	// before rename remove source archive
	removeFile(archiveName);

	// rename temp archive to source
	int ret = tas::renameFile(archiveNameTemp, archiveName);
	if(!ret)
		return 0;

	// reopen archive
	close();
	ret = open(archiveName);
	if(!ret)
		return 0;

	safe_delete_array(fileHeads);
	safe_delete(log);
	return 1;
}

int Archive::fileExist(const String& file)
{
	forn(_m->fileList.size())
	if(stwpat(_m->fileList[i]->name.p(), file.p()))
		return 1;
	return 0;
}

String Archive::getLastFile()
{
	return _m->lastFile;
}

int Archive::list()
{
	return _m->list();
}

int Archive::impl::list()
{
#define SPLIT_LINE \
	log->setConsoleOutput(0); \
	log->writeLinef(splitLineLog); \
	log->setConsoleOutput(1); \
	stdprintf("%s\n", splitLine);

	char splitLine[35] = {0};
	char splitLineLog[35] = {0};
	forn(34)
	{
		splitLine[i] = 0xC4;
		splitLineLog[i] = 0x2D;
	}
	forn(2)
	{
		splitLine[i + 16] = 0x20;
		splitLineLog[i + 16] = 0x20;
	}

	log->setConsoleOutput(1);
	log->writeLinef("%16s  %u.%u.%u\n", "Archivarius",
	                TAA_VERSION_MAJOR, TAA_VERSION_MINOR, TAA_VERSION_PATCH);

	log->writeLinef("%16s  %s", "Size", "Name");
	SPLIT_LINE;

	convertDecimalSpace(&buffer, archiveSize);
	logWriteLinef("%16ls  %ls", buffer.p(), archive.getName().p());
	convertDecimalSpace(&buffer, totalFileHeadSize);
	log->writeLinef("%16ls  %s\n", buffer.p(), "heads");
	log->writeLinef("%16s  %s", "Files", "list");

	SPLIT_LINE;
	forn(fileList.size())
	{
		convertDecimalSpace(&buffer, fileList[i]->size);
		logWriteLinef("%16ls  %ls", buffer.p(), fileList[i]->name.p());
	}

	SPLIT_LINE;
	convertDecimalSpace(&buffer, sourceFileSize);
	log->writeLinef("%16ls  %s", buffer.p(), "Total size");
	log->writeLinef("%16u  %s", fileList.size(), "Total count");

	if(encodeMethod)
	{
		if(encodeMethod == 1)
			bufferu = "LZRE";
		else if(encodeMethod == 2)
			bufferu = "CM";
		log->writeLinef("\n%16s  %s", bufferu.p(), "Compression");

		SPLIT_LINE;
		if(encodeMethod == 1)
		{
			log->writeLinef("%16u  %s", encodeLevel, "Level");
			convertDecimalSpace(&buffer, dictSize);
			log->writeLinef("%16ls  %s", buffer.p(), "Dictionary");
			bufferu.format("[%u, %u]", matchMin, matchMax);
			log->writeLinef("%16s  %s", bufferu.p(), "Match");
			log->writeLinef("%16u  %s", encodeThreads, "Threads");
		}
		else if(encodeMethod == 2)
		{
			log->writeLinef("%16u  %s", encodeLevel, "Level");
			convertDecimalSpace(&buffer, dictSize);
			log->writeLinef("%16ls  %s", buffer.p(), "Dictionary");
			log->writeLinef("%16u  %s", matchMax, "Context");
		}

		float ratio = (float) archiveSize / sourceFileSize;
		log->writeLinef("%16.2f  %s", ratio, "Ratio");
	}

	log->writeLinef("\n%16s  %s", "Extra", "Options");
	SPLIT_LINE;
	switch(cryptState)
	{
	case 0:
		bufferu = "None";
		break;
	case 1:
		bufferu = "Names";
		break;
	case 2:
		bufferu = "Data";
		break;
	case 3:
		bufferu = "All";
		break;
	}
	log->writeLinef("%16s  %s", bufferu.p(), "Encryption");

	if(encodeBigraph == 1)
		bufferu = "Bigraph";
	else
		bufferu = "None";
	log->writeLinef("%16s  %s", bufferu.p(), "Preprocess");

	return 1;
}

int Archive::createLog(const String& logName)
{
	safe_delete(_m->log);
	_m->log = new TextStream(logName.p(), 0);
	if(!_m->log->state())
		return 0;
	return 1;
}

void Archive::setValue(uint_t value, byte type)
{
	if(type == 0)
		_m->encodeMethod = value;
	if(type == 1)
		_m->encodeLevel = value;
	if(type == 2)
		_m->encodeThreads = value;
	if(type == 3)
		_m->dictSize = value;
	if(type == 4)
		_m->matchMax = value;
	if(type == 5)
		_m->matchCycles = value;
	if(type == 9)
		_m->matchMin = value;
	if(type == 20)
		_m->matchRolz = clamp(value, 3, 0);
	if(type == 21)
		_m->encodeCmix = value;
	if(type == 15)
		_m->encodeSuffix = value;
	if(type == 6)
	{
		if(_m->cryptString.capacity() == 0)
			_m->cryptString.reserve(1024);
		_m->cryptString.clear();
		_m->cryptString = reinterpret_cast<wchar_t*>(value);
		_m->cryptLength = _m->cryptString.length();
		_m->cryptString.resize(_m->cryptLength + 32); // fill nulls
		_m->cryptString.resize(_m->cryptLength); // restore size
		if(_m->cryptLength % 16)
			_m->cryptLength += 16 - (_m->cryptLength % 16);
	}
	if(type == 7)
	{
		_m->cutPaths = (Array<uint>*) value;
	}
	if(type == 8)
	{
		_m->event_callback = (int(*)(wint*)) value;
	}
	if(type == 10)
		_m->decPath = reinterpret_cast<wchar_t*>(value);
	if(type == 11)
		_m->savePath = value;
	if(type == 12)
		_m->extractMode = value;
	if(type == 14)
		_m->pathDelete = value;
	if(type == 16)
		_m->encodeBigraph = value;
	if(type == 17)
		_m->skipFileLen = value;
	if(type == 18)
		_m->pathArchive = reinterpret_cast<wchar_t*>(value);
	if(type == 19)
		_m->cryptState = value;
}

uint_t Archive::getValue(byte type)
{
	switch(type)
	{
	case 0:
		return (uint_t) &_m->fileList;
	case 1:
		return (uint_t) &_m->archive.getName();
	case 2:
		return (uint_t) _m->encodeMethod;
	case 3:
		return (uint_t) _m->encodeLevel;
	case 4:
		return (uint_t) _m->ratioIn;
	case 5:
		return (uint_t) _m->cryptState;
	case 6:
		return (uint_t) _m->encodeBigraph;
	case 7:
		return (uint_t) _m->sourceFileSize;
	case 8:
		return (uint_t) _m->arcHead.filesCount;
	case 9:
		return (uint_t) _m->arcHead.dataSize;
	case 10:
		return (uint_t) _m->cryptString.p();
	case 11:
		return (uint_t) _m->dictSize;
	case 12:
		return (uint_t) _m->matchMin;
	case 13:
		return (uint_t) _m->matchMax;
	case 14:
		return (uint_t) _m->matchRolz;
	case 15:
		return (uint_t) _m->encodeThreads;
	case 16:
		return (uint_t) _m->encodeCmix;
	}
	return 0;
}

byte Archive::getError()
{
	return _m->errorId;
}

String Archive::getErrorStr()
{
	return _m->errorStr;
}

String Archive::getName()
{
	return _m->archive.getName();
}

int Archive::impl::sortFileExt(StringArray& filesIn, StringArray& filesOut)
{
	byte haveExt = 0;
	uint ext[2] = {0}; // offset [.]ext
	byte* proc = new byte[filesIn.size()];
	forn(filesIn.size())
	proc[i] = 0;
	String files[2];

	forn(filesIn.size())
	{
		if(proc[i]) continue;
		proc[i] = 1;

		filesOut.push_back(filesIn[i]);

		haveExt = 1;
		ext[0] = filesIn[i].findr(0x2E);
		if(ext[0] == MAX_UINT32)
			haveExt = 0;
		ext[0]++;
		if(haveExt)
		{
			files[0] = filesIn[i].substr(ext[0]);
			files[0].tolower();
		}

		for(uint u = i + 1; u < filesIn.size(); u++)
		{
			if(proc[u]) continue;
			ext[1] = filesIn[u].findr(0x2E);
			if(!haveExt and ext[1] == MAX_UINT32)
			{
				filesOut.push_back(filesIn[u]);
				proc[u] = 1;
			}
			else if(haveExt and ext[1] != MAX_UINT32)
			{
				ext[1]++;
				files[1] = filesIn[u].substr(ext[1]);
				files[1].tolower();
				// if(stdcmp(filesIn[i].p() + ext[0], filesIn[u].p() + ext[1]))
				if(files[0] == files[1])
				{
					filesOut.push_back(filesIn[u]);
					proc[u] = 1;
				}
			}
		}
	}
	safe_delete_array(proc);
	return 1;
}

ICoder* Archive::impl::createMethod(byte modew)
{
#ifdef TAA_ARCHIVARIUS_COMPRESS
	byte encm = !modew ? 1 : 2;

	if(encodeMethod == 1)
	{
		LzreCoder* lzre = new LzreCoder;

		LzreParameters params;
		params.mode = encm;
		params.dictionary = dictSize;
		params.minMatch = matchMin;
		params.maxMatch = matchMax;
		params.level = encodeLevel;
		params.cycles = matchCycles;
		params.suffix = encodeSuffix;
		params.threads = encodeThreads;
		params.cmix = encodeCmix;

		if(matchRolz)
			params.rolz = 64 << matchRolz - 1;

		lzre->initialise(&params);

		dictSize = params.dictionary;
		matchMin = params.minMatch;
		matchMax = params.maxMatch;
		matchCycles = params.cycles;
		encodeSuffix = params.suffix;
		encodeThreads = params.threads;
		matchRolzOut = params.rolz;

		return lzre;
	}
#if TAA_PLATFORM_TYPE == TAA_PLATFORM_DESKTOP
	else if(encodeMethod == 2)
	{
		CmCoder* cmix = new CmCoder;

		CmParameters params;
		params.dictionary = dictSize;
		params.context = matchMax;
		params.level = encodeLevel;

		cmix->initialise(&params);

		dictSize = params.dictionary;
		matchMax = params.context;

		return cmix;
	}
#endif
#endif
	return 0;
}

int Archive::impl::getTime()
{
	wint msec = timer.getMilliseconds();
	mseconds = msec % 1000;
	seconds = (msec / 1000) % 60;
	minutes = msec / 60000;
	msecondsp = mseconds - mseconds % 100;
	return 1;
}

int Archive::impl::logWriteLinef(const char* fmty, ...)
{
	if(!fmty or *fmty == '\0') return 0;
	static wchar_t fmtw[1024];
	const wchar_t* fmt = fmtw;
	uint len = stdlen((char*)fmty);
	forn(len) fmtw[i] = fmty[i];
	fmtw[len] = 0;
	va_list args;
	va_start(args, fmt);
	uint size = vswprintf(0, 0, fmt, args);
	if(!size) return 0;
	formated.resize(size);
	size = vswprintf(formated.p(), size+1, fmt, args);
	va_end(args);
	int consoleOut = log->getConsoleOutput();
	if(consoleOut)
		stdprintf("%ls\n", formated.p());
	log->setConsoleOutput(0);
	log->writeLine(getUnicode(formated));
	if(consoleOut)
		log->setConsoleOutput(1);
	return 1;
}

char* Archive::impl::getUnicode(const String& wides)
{
	bufferu.assign(wides.p(), wides.length());
	return bufferu.p();
}

void Archive::impl::logTime(uint filesCount)
{
	uint divn = 0;
	uint remainder = 0;
	archiveHeadSizeTotal = archiveSize - arcHead.dataSize;
	log->writeLinef("\n%-12s  %u", "Files count", filesCount);
	convertDecimalSpace(&buffer, sourceFileSize);
	log->writeLinef("%-12s  %ls", "Total size", buffer.p());
	convertDecimalSpace(&buffer, archiveSize);
	log->writeLinef("%-12s  %ls", "Archive size", buffer.p());
	convertDecimalSpace(&buffer, totalFileHeadSize);
	log->writeLinef("%-12s  %ls", "Heads size", buffer.p());

	char spc[12] =
	{
		0x42, 0x00, 0x00, // B
		0x4B, 0x62, 0x00, // Kb
		0x4D, 0x62, 0x00, // Mb
		0x47, 0x62, 0x00  // Gb
	};

	if(encodeMethod)
	{
		float ratio = (float) arcHead.dataSize / sourceFileSize;
		if(encodeMethod == 1)
			bufferu = "LZRE";
		if(encodeMethod == 2)
			bufferu = "CM";

		log->writeLinef("\n%-12s  %s", "Compression", bufferu.p());
		log->writeLinef("%-12s  %u", "Level", encodeLevel);

		wint dictsz = 1 << bitGreat(dictSize - 1);
		divn = convertBytesToDecimal(dictsz, remainder);
		log->writeLinef("%-12s  %u %s", "Dictionary", static_cast<uint>(dictsz), spc + divn * 3);

		if(encodeMethod == 1)
		{
			log->writeLinef("%-12s  [%u, %u]", "Match", matchMin, matchMax);
			if(matchCycles)
				log->writeLinef("%-12s  %u", "Cycles", matchCycles);
			if(matchRolzOut)
				log->writeLinef("%-12s  %u", "Rolz", matchRolzOut);
			if(encodeSuffix)
				log->writeLinef("%-12s  %u", "Suffix", encodeSuffix);
			log->writeLinef("%-12s  %u", "Threads", encodeThreads);
			if(encodeCmix)
				log->writeLinef("%-12s  mixing", "Context");
		}
		else if(encodeMethod == 2)
		{
			log->writeLinef("%-12s  %u", "Context", matchMax);
		}
		log->writeLinef("%-12s  %.2f", "Ratio", ratio);
	}

	switch(cryptState)
	{
	case 0:
		bufferu = "None";
		break;
	case 1:
		bufferu = "Names";
		break;
	case 2:
		bufferu = "Data";
		break;
	case 3:
		bufferu = "All";
		break;
	}
	log->writeLinef("\n%-12s  %s", "Encryption", bufferu.p());

	if(encodeBigraph == 1)
		bufferu = "Bigraph";
	else
		bufferu = "None";
	log->writeLinef("%-12s  %s", "Preprocess", bufferu.p());

	log->setConsoleOutput(0);
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = GetCurrentProcess();
	if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		wint memsize = pmc.PeakWorkingSetSize;
		divn = convertBytesToDecimal(memsize, remainder);
		log->writeLinef("\n%-12s  %u.%u %s", "Memory", static_cast<uint>(memsize), remainder / 104,  spc + divn * 3);
	}
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
	struct rusage usage;
	if(!getrusage(RUSAGE_SELF, &usage))
	{
		wint memsize = usage.ru_maxrss * KB;
		divn = convertBytesToDecimal(memsize, remainder);
		log->writeLinef("\n%-12s  %u.%u %s", "Memory", static_cast<uint>(memsize), remainder / 104,  spc + divn * 3);
	}
#endif

	// speed byte / seconds
	wint past = timer.getMilliseconds() + 1;
	wint speed = sourceFileSize * 1000 / past;
	divn = convertBytesToDecimal(speed, remainder);
	log->writeLinef("%-12s  %u.%u %s/s", "Speed", static_cast<uint>(speed), remainder / 104,  spc + divn * 3);

	log->setAppendNewLine(0);
	uint minutes = past / 60000;
	uint seconds = (past / 1000) % 60;
	uint mseconds = past % 1000;
	log->writeLinef("%-12s  %02u:%02u:%03u", "Time", minutes, seconds, mseconds);
}

}