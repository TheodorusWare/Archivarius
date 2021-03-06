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
#include <Widget/Keyboard.h>

namespace tas
{

Keyboard::Keyboard()
{
	forn(256)
	keyMap[i] = 0;
}

Keyboard::~Keyboard()
{
}

byte Keyboard::getKeyState(uint key)
{
	byte state = KEY_STATE(key);
	keyMap[key] = state;
	return state;
}

byte Keyboard::setKeyState(uint key, byte state)
{
	keyMap[key] = state;
	return state;
}

byte Keyboard::getKeyPressed(uint key)
{
	byte state = KEY_STATE(key);
	byte ret = (!keyMap[key] and state);
	keyMap[key] = state;
	return ret;
}

byte Keyboard::getKeyReleased(uint key)
{
	byte state = KEY_STATE(key);
	byte ret = (keyMap[key] and !state);
	keyMap[key] = state;
	return ret;
}

}