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
#ifndef _TAH_Thread_h_
#define _TAH_Thread_h_

/// supporting winapi threads
#ifdef TAA_ARCHIVARIUS_THREAD
#include <Common/platform/system.h>
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <process.h>

/// atomic operations for read, write shared variables from other threads
#define atomic_get(o)    InterlockedExchangeAdd((long*)&o, 0)
#define atomic_set(o, v) InterlockedExchange((long*)&o, v)
/// o == c ? o = n
#define atomic_set_cmp(o, c, n) InterlockedCompareExchange((long*)&o, n, c)
#define atomic_incv(o, v) InterlockedExchangeAdd((long*)&o, v)
#define atomic_inc(o) InterlockedIncrement((long*)&o)
#define atomic_dec(o) InterlockedDecrement((long*)&o)

#define thread_create(thread, arg) (uint*) _beginthreadex(0, 0, thread, arg, 0, 0);
#define thread_createex(thread, arg, if) (uint*) _beginthreadex(0, 0, thread, arg, if, 0);
#define thread_wait(thread, timeout) WaitForSingleObject(thread, timeout);
#define thread_close(thread) CloseHandle(thread);
#define thread_close_wait(thread, timeout) WaitForSingleObject(thread, timeout); CloseHandle(thread);
#endif
#else
#define atomic_get(o) o
#define atomic_set(o, v)
#define atomic_set_cmp(o, c, n)
#define atomic_incv(o, v)
#define atomic_inc(o)
#define atomic_dec(o)
#endif

#endif