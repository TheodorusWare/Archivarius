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
#include <Common/Registry.h>

namespace tas
{

Registry::Registry()
{
	hKey = 0;
}

Registry::~Registry()
{
	close();
}

int Registry::open(HKEY key, const String& subKey, byte load)
{
	if(RegOpenKeyEx(key, subKey.p(), 0, load ? KEY_QUERY_VALUE : KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;
	return 1;
}

int Registry::create(HKEY key, const String& subKey)
{
	if(RegCreateKey(key, subKey.p(), &hKey) != ERROR_SUCCESS)
		return 0;
	return 1;
}

int Registry::remove(HKEY key, const String& subKey)
{
	if(RegDeleteKey(key, subKey.p()) != ERROR_SUCCESS)
		return 0;
	return 1;
}

int Registry::close()
{
	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = 0;
	}
	return 1;
}

int Registry::getValue(const String& value, byte* data, uint* dataSize, uint* type)
{
	if(!hKey)
		return 0;
	if(RegQueryValueEx(hKey, value.p(), NULL, (LPDWORD) type, data, (LPDWORD) dataSize) != ERROR_SUCCESS)
		return 0;
	return 1;
}

int Registry::setValue(const String& value, uint type, byte* data, uint dataSize)
{
	if(!hKey)
		return 0;
	if(RegSetValueEx(hKey, value.p(), 0, type, data, dataSize) != ERROR_SUCCESS)
		return 0;
	return 1;
}

}