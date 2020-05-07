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
#ifdef TAA_ARCHIVARIUS_THREAD
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
#include <Common/platform/swindows.h>
#include <Common/Thread.h>
static uint WINAPI search_thread(void* param);
#define RETT uint WINAPI
#define ATOMIC Atomic
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
#include <pthread.h>
#include <unistd.h>
static void* search_thread(void* param);
#define RETT void*
#define ATOMIC Atomix
#endif

static ATOMIC threads_state[3];

class LzreThread
{
	byte threadsCount;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
	uint** threads;
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
	pthread_t* threads;
#endif

public:

	LzreThread()
	{
		threads = 0;
		threadsCount = 0;
	}

	~LzreThread()
	{
	}

	int initialise(byte threadn)
	{
		threadsCount = threadn;

		if(threadn == 0)
			return 0;

#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
		threads = new uint*[threadn];
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
		threads = new pthread_t[threadn];
#endif
		forn(threadn)
		{
			threads_state[i].a = TS_IDLE;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
			threads[i] = thread_create(search_thread, 0);
			assert(threads[i]);
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
			int err = pthread_create(&threads[i], 0, search_thread, 0);
			assert(err == 0);
#endif
		}
		return 1;
	}

	int uninitialise()
	{
		forn(threadsCount)
		{
			threads_state[i]--;
#if TAA_PLATFORM == TAA_PLATFORM_WINDOWS
			thread_close_wait(threads[i], 100);
#elif TAA_PLATFORM == TAA_PLATFORM_LINUX
			void* res = 0;
			pthread_join(threads[i], &res);
#endif
			threads[i] = 0;
		}
		safe_delete_array(threads);
		return 1;
	}
};

void findMatchThread(byte id);

RETT search_thread(void* param)
{
	uint id = 0;
	uint ids = 1;
	uint state = 0;
	while(1)
	{
		state = threads_state[id].get();
		if(state == TS_IDLE);
		elif(state == TS_RUN)
		{
			findMatchThread(ids);
			threads_state[id]--;
		}
		elif(state == TS_EXIT)
		break;
	}
	return 0;
}

#else
class LzreThread
{
public:
	int init(byte threadn) {}
	int deinit() {}
};

// dummy
struct Atomic
{
	Atomic() {}
	uint operator ++ (int) {}
	uint operator -- (int) {}
	uint operator =  (uint v) {}
	uint get() {}
};

static Atomic threads_state[3];
#endif

#define LENGTH_CONTEXT_PREPARE \
if(match.type != MT_MATCH)\
{\
	lengthShift = maskf[contextBit[type] + 1] * (match.type - 1);\
	contextShift[type] += lengthShift;\
	lengthShiftType = maskf[contextBit[CT_LENGTH_TYPE] + 1] * (match.type - 1);\
	contextShift[CT_LENGTH_TYPE] += lengthShiftType;\
}

#define LENGTH_CONTEXT_UPDATE \
if(match.lengthShort)\
	pmLen = (pmLen << 1 | 1) & 15;\
else\
	pmLen = pmLen << 1 & 15;\
if(lengthShift)\
	contextShift[type] -= lengthShift,\
	contextShift[CT_LENGTH_TYPE] -= lengthShiftType;