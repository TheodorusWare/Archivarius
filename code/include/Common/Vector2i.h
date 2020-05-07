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
#ifndef _TAH_Vector2i_h_
#define _TAH_Vector2i_h_

#include <Common/Config.h>

namespace tas
{

class TAA_LIB Vector2i
{
public:
	int x, y;

	Vector2i ();
	Vector2i (int scalar);
	Vector2i (int x, int y);
	Vector2i (const Vector2i& v);

	Vector2i operator = (Vector2i vec);
	Vector2i operator = (int);

	Vector2i operator + (Vector2i vec);
	Vector2i operator - (Vector2i vec);
	Vector2i operator * (Vector2i vec);
	Vector2i operator / (Vector2i vec);
	Vector2i operator - ();

	Vector2i operator += (Vector2i vec);
	Vector2i operator -= (Vector2i vec);
	Vector2i operator *= (Vector2i vec);
	Vector2i operator /= (Vector2i vec);

	bool operator == (Vector2i vec);
	bool operator != (Vector2i vec);
	bool operator >  (Vector2i vec);
	bool operator <  (Vector2i vec);
	bool operator >= (Vector2i vec);
	bool operator <= (Vector2i vec);

	int& operator[] (int i);
};

}

#endif