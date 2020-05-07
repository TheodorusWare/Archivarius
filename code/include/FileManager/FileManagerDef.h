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
#ifndef _TAH_GuiFileManagerDef_h_
#define _TAH_GuiFileManagerDef_h_

#define PROGRAM_TITLE  L"Archivarius"
#define ARCHIVE_EXT    L".arv"

/* Main actions for menu, toolbar and commands */
#define ID_APPEND       10
#define ID_EXTRACT      11
#define ID_TEST         12
#define ID_INFO         13
#define ID_DELETE       14

/* Menu items */
/* File */
#define ID_MF_OPEN      20
#define ID_MF_CLOSE     21
#define ID_MF_OPEN_LOG  22
#define ID_MF_OPTIONS   23
#define ID_MF_EXIT      24

/* Commands */
#define ID_MC_RENAME    30
#define ID_MC_UPDATE    31
#define ID_MC_SELADR    32
#define ID_MC_SELALL    33
#define ID_MC_NEWDIR    34
#define ID_MC_COPY      35
#define ID_MC_PASTE     36

/* View */
#define ID_MV_NAME      40
#define ID_MV_TYPE      41
#define ID_MV_TIME      42
#define ID_MV_SIZE      43

/* Help */
#define ID_MH_MANUAL    45
#define ID_MH_ABOUT     46

/* Widget controls id */
#define ID_WC_TOOLBAR    2
#define ID_WC_STATUSBAR  3
#define ID_WC_ADDRESS    4
#define ID_WC_FILELIST   5

/* Commands */
#define FMC_ACTIVATE            50
#define FMC_DEACTIVATE          51
#define FMC_SIZE                52
#define FMC_LIST_UPDATE         53
#define FMC_LIST_SELECT         54
#define FMC_LIST_BACK           55
#define FMC_LIST_EDIT           56
#define FMC_LIST_COLUMN         57
#define FMC_ADDRESS_SELECT      58
#define FMC_ADDRESS_EDITED      59
#define FMC_ADDRESS_KILLFOCUS   60

#define FMC_EXIT                99

/* Other stuff */
#define BUFFER_SIZE   2048

// exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#include <Commdlg.h>
#include <Shellapi.h>
#include <Shlobj.h>

#endif