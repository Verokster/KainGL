/*
	MIT License

	Copyright (c) 2018 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"
#include "Config.h"
#include "Glib.h"

TCHAR iniFile[MAX_PATH];

DWORD configGlVersion;
DWORD configGlFiltering;

BOOL configDisplayWindowed;
Resolution configDisplayResolution;
BOOL configDisplayAspect;

BOOL configFpsCounter;
FLOAT configFpsLimit;

BOOL configOtherSkipIntro;

namespace Config
{
	VOID Load()
	{
		GetModuleFileName(hDllModule, iniFile, MAX_PATH);
		*strrchr(iniFile, '.') = NULL;
		strcat(iniFile, ".ini");
	}

	DWORD __fastcall Get(const CHAR* section, const CHAR* key, DWORD def)
	{
		return GetPrivateProfileInt(section, key, def, iniFile);
	}

	BOOL __fastcall Set(const CHAR* section, const CHAR* key, DWORD value)
	{
		TCHAR res[20];
		sprintf(res, "%d", value);
		return WritePrivateProfileString(section, key, res, iniFile);
	}
}