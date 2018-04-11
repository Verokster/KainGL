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

#pragma once
#include "windows.h"

#define CONFIG_GL "OPENGL"
#define CONFIG_GL_VERSION "Version"
#define CONFIG_GL_FILTERING "Filtering"

#define CONFIG_DISPLAY "DISPLAY"
#define CONFIG_DISPLAY_WINDOWED "Windowed"
#define CONFIG_DISPLAY_RESOLUTION "Resolution"
#define CONFIG_DISPLAY_ASPECT "Aspect"

#define CONFIG_FPS "FPS"
#define CONFIG_FPS_LIMIT "Limit"
#define CONFIG_FPS_COUNTER "Counter"

#define CONFIG_OTHER "OTHER"
#define CONFIG_OTHER_SKIP_INTRO "SkipIntro"

extern TCHAR iniFile[];

extern DWORD configGlVersion;
extern DWORD configGlFiltering;

extern BOOL configDisplayWindowed;
extern Resolution configDisplayResolution;
extern BOOL configDisplayAspect;

extern FLOAT configFpsLimit;
extern BOOL configFpsCounter;

extern BOOL configOtherSkipIntro;

namespace Config
{
	VOID Load();
	DWORD __fastcall Get(const CHAR* section, const CHAR* key, DWORD def);
	BOOL __fastcall Set(const CHAR* section, const CHAR* key, DWORD value);
}