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
#ifndef _TAH_Archive_h_
#define _TAH_Archive_h_

#include <Common/Config.h>
#include <Common/StringArray.h>

namespace tas
{

class TAA_LIB Archive
{
public:
	Archive();
	~Archive();

	/** Create new archive.
	  * @param name New archive name.
	  * @return 1 success else 0.
	  */
	int create(const String& name);

	/** Load exist archive.
	  * @param name Archive name.
	  * @param mode 1 force, 2 skip check password.
	  * @return 1 success else 0.
	  */
	int open(const String& name, byte mode = 0);

	/** Close archive.
	  * @param mode 0 save crypt string, 1 delete crypt.
	  * @return 1 success else 0.
	  */
	int close(byte mode = 0);

	/** Check crypt string.
	  * @return 1 success else 0.
	  */
	int checkCrypt();

	/** Save current crypt string.
	  * @return 1 success else 0.
	  */
	int saveCrypt();

	/** Reset crypt string to default.
	  * @return 1 success else 0.
	  */
	int resetCrypt();

	/** Append files to archive.
	  * @param files Appended files.
	  * @return Positive value if success, else nil.
	  */
	int append(StringArray& files);

	/** Extract one or all files from archive.
	  * @param files Extracted files.
	  *        If files is 0 proccessed all files.
	  *        Type void* maybe StringArray* or char*,
	  *        see impl::extractMode.
	  * @return 1 success else 0.
	  */
	int extract(void* files);

	/** Delete files from archive.
	  * @param files Deleted files.
	  * @return Positive value if success, else nil.
	  */
	int remove(StringArray& files);

	/** Rename single folder or many files.
	  * @param files Rename folder or files.
	  * @remarks
	  *   For build files array use next rules.
	  *     A\ B
	  *     rename folder A to B
	  *     A1 B1 [A2 B2 ..]
	  *     rename file A1 to B1, A2 to B2 ..
	  *   Count items in files array must be even.
	  * @return Positive value if success, else nil.
	  */
	int rename(StringArray& files);

	/** If file exist return 1, else 0.
	  */
	int fileExist(const String& file);

	/** Return last extracted file.
	  */
	String getLastFile();

	/** Extract list archive contents.
	  */
	int list();

	/** Create log file.
	  * @param logName File name.
	  */
	int createLog(const String& logName);

	/** Set, get other values.
	  * @param value Main value.
	  * @param type Value type.
	  * @return need value.
	  * @remark value may be c string with type char*.
	  */
	void setValue(uint_t value, byte type);
	uint_t getValue(byte type);

	/** Return type of last error.
	  */
	byte getError();

	/** Return last error description.
	  */
	String getErrorStr();

	/** Return archive file name.
	  */
	String getName();

private:
	struct impl;
	impl* _m;
};

}

#endif