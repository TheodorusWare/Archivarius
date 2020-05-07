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
#include <Common/Vector2i.h>
#include <Common/Math.h>

namespace tas
{

Vector2i::Vector2i()
{
	x = 0;
	y = 0;
}

Vector2i::Vector2i(int _x, int _y)
{
	x = _x;
	y = _y;
}

Vector2i::Vector2i(int scalar)
{
	x = scalar;
	y = scalar;
}

Vector2i::Vector2i(const Vector2i& v)
{
	x = v.x;
	y = v.y;
}

Vector2i Vector2i::operator = (Vector2i vec)
{
	x = vec.x;
	y = vec.y;
	return *this;
}

Vector2i Vector2i::operator = (int scalar)
{
	*this = Vector2i(scalar);
	return *this;
}

Vector2i Vector2i::operator + (Vector2i vec)
{
	return Vector2i (x + vec.x,	y + vec.y);
}

Vector2i Vector2i::operator - (Vector2i vec)
{
	return Vector2i (x - vec.x, y - vec.y);
}

Vector2i Vector2i::operator * (Vector2i vec)
{
	return Vector2i (x * vec.x, y * vec.y);
}

Vector2i Vector2i::operator / (Vector2i vec)
{
	return Vector2i(x / vec.x, y / vec.y);
}

Vector2i Vector2i::operator += (Vector2i vec)
{
	x += vec.x;
	y += vec.y;
	return *this;
}

Vector2i Vector2i::operator -= (Vector2i vec)
{
	x -= vec.x;
	y -= vec.y;
	return *this;
}

Vector2i Vector2i::operator *= (Vector2i vec)
{
	x *= vec.x;
	y *= vec.y;
	return *this;
}

Vector2i Vector2i::operator /= (Vector2i vec)
{
	x /= vec.x;
	y /= vec.y;
	return *this;
}

Vector2i Vector2i::operator - ()
{
	return Vector2i(-x, -y);
}

bool Vector2i::operator == (Vector2i vec)
{
	return (x == vec.x && y == vec.y);
}

bool Vector2i::operator != (Vector2i vec)
{
	return (x != vec.x || y != vec.y);
}

bool Vector2i::operator > (Vector2i vec)
{
	return (x > vec.x && y > vec.y);
}

bool Vector2i::operator >= (Vector2i vec)
{
	return (x >= vec.x && y >= vec.y);
}

bool Vector2i::operator < (Vector2i vec)
{
	return (x < vec.x && y < vec.y);
}

bool Vector2i::operator <= (Vector2i vec)
{
	return (x <= vec.x && y <= vec.y);
}

int& Vector2i::operator[](int i)
{
	return *(&x+i);
}

}