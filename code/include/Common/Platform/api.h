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
#ifndef _TAH_platform_api_h_
#define _TAH_platform_api_h_

#define API_NAME "Archivarius"

#define TAA_VERSION_MAJOR 0
#define TAA_VERSION_MINOR 3
#define TAA_VERSION_PATCH 1
#define TAA_VERSION (TAA_VERSION_MAJOR << 8 | TAA_VERSION_MINOR << 4 | TAA_VERSION_PATCH)

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS

#if defined(TAA_BUILD_LIB)
#define TAA_LIB __declspec(dllexport)
#elif defined(TAA_LINK_LIB)
#define TAA_LIB __declspec(dllimport)
#else
#define TAA_LIB
#endif

#define TAA_LIB_EXP __declspec(dllexport)
#define TAA_LIB_IMP __declspec(dllimport)

#elif TAA_PLATFORM == TAA_PLATFORM_LINUX

#if __GNUC__ >= 4
#if defined(TAA_BUILD_LIB) || defined(TAA_LINK_LIB)
#define TAA_LIB __attribute__((visibility("default")))
#else
#define TAA_LIB __attribute__ ((visibility ("hidden")))
#endif
#define TAA_LIB_EXP __attribute__((visibility("default")))
#define TAA_LIB_IMP __attribute__((visibility("default")))
#else
#define TAA_LIB
#define TAA_LIB_EXP
#define TAA_LIB_IMP
#endif

#endif
#endif