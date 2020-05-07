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
#include <Common/platform/swindows.h>
#include <Common/Timer.h>

namespace tas
{

Timer::~Timer()
{
}

Timer::Timer()
{
	LARGE_INTEGER frq;
	QueryPerformanceFrequency(&frq);
	frmFrq = frq.QuadPart; /// 3 222 705
	startTime = 0;
	reset();
}

void Timer::reset()
{
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
	startTime = start.QuadPart;
}

wint Timer::getMilliseconds()
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	double past = (double)(end.QuadPart - startTime) / (double)frmFrq;
	return past * 1000.0;
}

wint Timer::getMicroseconds()
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	double past = (double)(end.QuadPart - startTime) / (double)frmFrq;
	return past * 1000000.0;
}

}