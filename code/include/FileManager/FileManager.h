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
#ifndef _TAH_GuiFileManager_h_
#define _TAH_GuiFileManager_h_

#include <Common/Config.h>
#include <Common/StringArray.h>
#include <Common/Vector2i.h>
#include <Compress/Archive.h>
#include <Widget/Application.h>
#include <Widget/Window.h>
#include <Widget/Checkbox.h>
#include <Widget/Combobox.h>
#include <Widget/Statusbar.h>
#include <Widget/Toolbar.h>
#include <Widget/Listview.h>
#include <Widget/Imagelist.h>
#include <Widget/DragDrop.h>
#include <Widget/Keyboard.h>
#include <FileManager/FileSearch.h>

namespace tas
{

/// Program settings
struct ArchiverSettings
{
	byte level; /// encode [0, 6]
	byte method;
	byte dictSize;
	byte matchMin;
	byte matchMax;
	byte threads;
	byte bigraph;

	byte windowMaximize; /// program [7, 14]
	byte windowAlign;
	byte fullRowSelect;
	byte cryptFile;
	byte showPass;
	byte filePath; /// decode
	byte sortType;
	byte sortMode;

	ArchiverSettings();
	void init();
	byte& operator[](int i);
};

struct ArchiveFileList
{
	String name;
	wint size;
	wint pos;
	uint crc;
	uint time[2];
};

class FileManager
{
public:
	byte mFileSystemType;  /// 1 file system; 2 archive
	byte mFileSystemLevel; /// 0 root, else in folder
	byte mArchiveLevel;    /// 0 root, else in folder
	byte mAddressItemCount;   /// current
	byte mAddressItemCurrent; /// additional item with current address
	byte mDriveCount;         /// disk drives
	byte mButtonUp; /// have
	byte mAppendType; /// 0 def, 1 drag drop
	/** bit flags. low 2 bit file path: 0 nothing, 1 from dialog, 2 force relative
		 single 3 bit: 4 use file list
		 single 8 bit test archive */
	byte mUnpackType;
	byte mListEdit; /// 1 user now edit file list item
	String mAddressCurrent; /// address file sys
	String mAddressArchive; /// address\ #
	String mAddressEdit;    /// address
	String mAddressUp;   /// address up
	String mBuffer;  /// string buffer
	String mBufferw; /// string buffer
	String mPassword;
	String mDirectoryUnpack; /// file launch
	String mArchiveDirectory;
	String mArchiveName;
	String mArchiveBackup;

	/** Interface */
	GuiApplication * mApplication; /// not change prefix
	Window         * mWindow;
	Toolbar        * mToolbar;
	Statusbar      * mStatusbar;
	Combobox       * mgAddress;
	Listview       * mgFileList;

	/** Program settings */
	ArchiverSettings mArchiverSettings;

	/** Localisation */
	StringArray mLocalisation;
	String mLocalisationId;

	Vector2i mWindowPos;
	Vector2i mWindowSize;
	Vector2i mAddressPos;
	Vector2i mAddressSize;
	Vector2i mFileListPos;
	Vector2i mFileListSize;
	Vector2i mButtonSize; /// ok, cancel

	Imagelist mToolbarImage;
	Imagelist mAddressImage;
	Imagelist mFileListImage;

	Keyboard mKeyboard;
	Array<half> mCommands; /// manager commands

	/** Search module */
	SearchFiles mSearch;
	StringArray mFiles;
	StringArray mFolders;

	/** File list search items */
	uint  mListSelect[3]; /// selected directories, files, total
	int   mListIndex;     /// file list current index, start -1
	int   mItemIndex;     /// in files or folders array
	int   mItemType;      /// 0 stop, 1 folder, 2 file, 3 up
	wchar_t* mItemString;    /// only pointer to type 1, 2

	/** File list sorting */
	byte mSortType; /// index column [0, 3]
	byte mSortMode; /// 0 down, 1 up [1]
	byte mSortList; /// need update?

	Archive     mArchive;
	StringArray mArchiveFiles;
	Array<ArchiveFileList*>* mArchiveFileList;
	Array<ArchiveFileList*> mArchiveListCurrent; /// current file items in list

	/** DragDrop */
	DragDrop mDragDrop;

	void* mFontSystem; /// system font
	void* mFontLarge;  /// large font 12
	void* mImageLogo;  /// logotype

	uint mAddressTime[2]; /// last modified time
	uint mTimeUpd;
	uint mRootHash;
	byte mSystemSegregation;
	byte mStop;
	int mReturn;

	static FileManager* mPtr;

public:
	FileManager();
	~FileManager();
	bool create(const String& archiveFile);
	bool initialise();
	bool createMenu();
	bool createToolbar();
	bool createAddress();

	bool findLogicalDrives();
	bool initFileList();
	bool updateFileList();
	bool sortFileList();
	bool updateStatusbar();
	bool addressEdited();
	bool addressSelected();
	bool listSelected();
	bool renameFile();
	bool openFolder();
	bool openFile();
	bool createFolder();
	bool deleteFiles();
	bool information();
	bool launchManual();
	bool launchLog();
	bool updateAddress();
	bool getFileType(String& file, String& type);

	bool readRegistry(byte state);
	bool readLocalisation();

	bool createContextMenu();
	bool copyClipboard();
	bool copyClipboardImpl(byte type);
	bool pasteClipboard();

	/** Get next selected item in file list.
		 Before first call set mListIndex to -1. */
	int getListSelect();

	/** Prepare append files to archive. */
	bool appendFiles();
	bool extractFiles();
	bool extractFilesSys();
	bool createArchive();
	int  extractArchive();
	bool prepareExtract();
	bool renameArchive();
	bool deleteArchive();
	bool testArchive();
	int  openArchive();
	bool closeArchive();
	bool closeArchiveMenu();
	bool openArchiveDialog();
	bool openFileArchive();
	bool copyClipboardArchive();
	bool searchArchiveFiles();
	bool updateFileListArchive();
	bool fileExistArchive(String& fileIn, byte copy);
	bool backupArchive(byte mode);
	bool queryPassword(byte type);
	bool statusArchive();

	int  callback(Widget* widget, uint message, uint_t param);
	void idle();
	void commands();
	void keyboard();
	void onButtons(uint id);
	void onSize();

	void setCommand(half cmd);

	int messageBox(const String& message, const String& caption, uint type);

	/** Singleton. */
	static FileManager* ptr();
};

#define FM FileManager::ptr()

}

#endif