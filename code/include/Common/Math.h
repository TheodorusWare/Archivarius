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
#ifndef _TAH_Math_h_
#define _TAH_Math_h_

#include <Common/Config.h>

namespace tas
{

/** Undefine early declared macro. */
#undef abs
#undef sin
#undef cos
#undef acos
#undef sqrt
#undef log2
#undef floor
#undef min
#undef max

/** Round float value.
  * @param v Round value.
  * @param n Count rounding signs after dot.
  * @return Rounded value.
  */
float TAA_LIB round(float v, byte n = 2);

/** Round float value to downward.
  * @param v Round value.
  * @return Rounded value.
  */
float TAA_LIB floor(float v);

/** Compare float values.
  * @param a First value.
  * @param b Second value.
  * @param n Count rounding signs after dot.
  * @return True for equal values, otherwise false.
  */
bool TAA_LIB compare(float a, float b, byte n = 2);

/** Clamp value to range [hi, low].
  * @param v   Clamp value.
  * @param hi  High range.
  * @param low Low range.
  * @return Clamped value.
  */
float TAA_LIB clamp(float v, float hi, float low);
int   TAA_LIB clamp(int   v,   int hi,   int low);

/** Test value in range [low, hi].
  * @param v   Test value.
  * @param hi  High range.
  * @param low Low range.
  * @return Positive value if v in range [low, hi], else 0.
  */
bool TAA_LIB range(int v, int hi, int low);

/** Compare values.
  * @param a First value.
  * @param b Second value.
  * @return Positive value if a > b, negative a < b, else 0.
  */
int TAA_LIB compare(uint a, uint b);

/** Retrieve minimum, maximum value of two values.
  */
int TAA_LIB min(int a, int b);
int TAA_LIB max(int a, int b);

/** Calculate value sign.
  */
float TAA_LIB sign(float v);
int   TAA_LIB sign(int v);

/** Retrieve absolute value.
  */
float  TAA_LIB abs(float  v);
double TAA_LIB abs(double v);
int    TAA_LIB abs(int v);

/** Calculate sine.
  * @param rad Angle in radians.
  */
float  TAA_LIB sin(float  rad);
double TAA_LIB sin(double rad);

/** Calculate cosine.
  * @param rad Angle in radians.
  */
float  TAA_LIB cos(float  rad);
double TAA_LIB cos(double rad);

/** Calculate arc cosine.
  * @param rad Angle in radians.
  */
float  TAA_LIB acos(float  rad);
double TAA_LIB acos(double rad);

/** Calculate square root.
  */
float  TAA_LIB sqrt(float  v);
double TAA_LIB sqrt(double v);

/** Retrieve mathematical constant Pi.
  */
float  TAA_LIB pi();
double TAA_LIB pii();

/** Convert degrees to radians.
  * @param v Degrees.
  * @return Radians.
  */
float TAA_LIB rad(float grad);

/** Convert radians to degrees.
  * @param v Radians.
  * @return Degrees.
  */
float TAA_LIB grad(float rad);

/** Calculate logarithm with base 2.
  */
float TAA_LIB log2(float v);

/** Calculate logarithm with base 2.
  * @remark From idTech4.
  */
int TAA_LIB log2i(float f);

/** Rush value a to b.
  * @param a Source value.
  * @param b Destination value.
  * @param sec Interval in seconds.
  * @param ft Frame time.
  * @return Updated value a.
  */
float TAA_LIB rush(float a, float b, float sec, float ft);

}

#endif