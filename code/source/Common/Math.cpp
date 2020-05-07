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
#include <Common/Math.h>
#include <math.h>

namespace tas
{

int _floor(float v)
{
	if(v < 0.0f)
		return (int) (v - 1.0f);
	else
		return (int) v;
}

int _round(float v, int n)
{
	if(!n) return _floor(v + 0.5f);
	float m = 1.0f;
	forn(n) m *= 10.0f;
	return _floor(v * m + 0.5f);
}

float round(float v, byte n)
{
	float m = 1.0f;
	forn(n) m *= 10.0f;
	return (float) floor(v * m + 0.5f) / m;
}

float floor(float v)
{
	if(v < 0.0f)
		return (int) (v - 1.0f);
	else
		return (int) v;
}

bool compare(float a, float b, byte n)
{
	/* float m = 1.0f;
	forn(n + 1) m *= 10.0f;
	float h = 1.0f / m;
	float d = abs(round(a, n) - round(b, n));
	return d < h; */
	return _round(a, n) == _round(b, n);
}

float clamp(float v, float hi, float low)
{
	if (v > hi)
		return hi;
	else if (v < low)
		return low;
	else
		return v;
}

int clamp(int v, int hi, int low)
{
	if (v > hi)
		return hi;
	else if (v < low)
		return low;
	else
		return v;
}

bool range(int v, int hi, int low)
{
	return low <= v and v <= hi;
}

int compare(uint a, uint b)
{
	if(a > b)
		return 1;
	if(a < b)
		return -1;
	return 0;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

float sign(float v)
{
	return v < 0.0f ? -1.0f : 1.0f;
}

int sign(int v)
{
	return v < 0.0f ? -1.0f : 1.0f;
}

float abs(float v)
{
	return v < 0.0f ? -v : v;
}

double abs(double v)
{
	return v < 0.0 ? -v : v;
}

int abs(int v)
{
	return v < 0 ? -v : v;
}

float sin(float rad)
{
	return (float)::sin((double)rad);
}

double sin(double rad)
{
	return ::sin(rad);
}

float cos(float rad)
{
	return (float)::cos((double) rad);
}

double cos(double rad)
{
	return ::cos(rad);
}

float acos(float rad)
{
	return (float)::acos((double) rad);
}

double acos(double rad)
{
	return ::acos(rad);
}

float sqrt(float v)
{
	return (float)::sqrt((double)v);
}

double sqrt(double v)
{
	return ::sqrt(v);
}

float pi()
{
	return 3.1415926f;
}

double pii()
{
	return 3.141592653589793;
}

float rad(float grad)
{
	return (grad * (pi() / 180.0f));
}

float grad(float rad)
{
	return (rad * (180.0f / pi()));
}

float log2(float v)
{
	return log(v) / 0.693147f;
}

int log2i(float f)
{
	return (*(int*)&f >> 23 & 0xFF) - 127;
}

float rush(float a, float b, float sec, float ft)
{
	const float mlt = 6.5f;
	float t = (float) ft * (mlt / sec);
	a += (b - a) * t;
	return a;
}

}