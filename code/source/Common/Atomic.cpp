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
#include <Common/Config.h>
#include <Common/Atomic.h>
#include <Common/Thread.h>

namespace tas
{

Atomic::Atomic()
{
	a = 0;
}

uint Atomic::operator ++ (int)
{
	return atomic_inc(a);
}

uint Atomic::operator -- (int)
{
	return atomic_dec(a);
}

uint Atomic::operator = (uint v)
{
	return atomic_set(a, v);
}

uint Atomic::get()
{
	return atomic_get(a);
}

}