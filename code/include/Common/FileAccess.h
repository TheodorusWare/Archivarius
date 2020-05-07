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
#ifndef _TAH_FileAccess_h_
#define _TAH_FileAccess_h_

#include <Common/Config.h>
#include <Common/StringArray.h>
#include <Common/UString.h>

namespace tas
{

/** @param pattern File, dir, maybe have wildards '*', '?', backslash '\'
  * @example "file", "dir", "*.fb2", "dir\\*", "dir\\*.fb2"
  */
TAA_LIB bool fileExist(const String& file);
TAA_LIB bool folderExist(const String& dir);
TAA_LIB wint fileSize(const String& file);
TAA_LIB bool fileName(const String& file, String& out);
TAA_LIB bool fileTime(const String& file, String& times, byte mode);
TAA_LIB bool fileTimeValue(const String& file, uint* times, byte mode);
TAA_LIB bool fileDosTimeValue(uint dosDate, uint* times);
/// mode: 1 conv sys time to local time, 0 not
TAA_LIB bool fileTimeString(uint* timei, String& times, byte mode = 1);
TAA_LIB bool localTimeString(UString& times);
TAA_LIB bool localTimeMd5String(UString& times);
/// mode: 0 file, 1 dir
TAA_LIB void createFolder(const String& dir, byte mode = 0);
TAA_LIB void removeFolder(const String& dir);
TAA_LIB bool removeFile(const String& file);
TAA_LIB bool removeFileShell(const String& file, bool recycleBin = 0);
TAA_LIB bool renameFile(const String& fileIn, const String& fileOut);
TAA_LIB void hideFile(const String& file, byte mode = 1);
TAA_LIB bool generateFileName(String& buffer, const String& base, byte mode = 0, byte num = 6, byte root = 1);
/** Search files in directorys.
  * @param pattern File, directory, maybe have wildards '*', '?'.
  * @param recurse Recursive search depth.
  * @param files[out] Output founded files.
  * @param dirs[out] Output founded directorys.
  * @return Positive when success, else nil.
  */
TAA_LIB byte findFiles(const String& pattern, byte recurse, StringArray& files, StringArray* dirs = 0);

}

#endif