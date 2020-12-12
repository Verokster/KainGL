/*
	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

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
#include "hooker.h"

#define f(a) (a + baseOffset)

extern CHAR kainDirPath[];
extern CHAR kainJamPath[];

struct LangFiles
{
	CHAR audioFile[MAX_PATH];
	CHAR voicesFile[MAX_PATH];
	CHAR interfaceFile[MAX_PATH];
	CHAR subtitlesFile[MAX_PATH];
};

struct FileHeader
{
	DWORD hash;
	DWORD size;
	DWORD offset;
};

extern LangFiles langFiles;
extern FileHeader* filesHeaders;
extern DWORD* filesCount;

extern FILE** filesHandlers;
extern CHAR* bigPathes[];

extern const CHAR* trailersList[2];

namespace Hooks
{
	extern HWND hMainWnd;

	extern DWORD sub_GetHash;

	BOOL Load();

	VOID Patch_Library(HOOKER);
	VOID Patch_System(HOOKER);
	VOID Patch_Timers(HOOKER);
	VOID Patch_Window(HOOKER);
	VOID Patch_Video(HOOKER);
	VOID Patch_Trailer(HOOKER);
	VOID Patch_Audio(HOOKER);
	VOID Patch_Mouse(HOOKER);
	VOID Patch_NoCD(HOOKER);
	VOID Patch_Language(HOOKER);
	VOID Patch_Zoom(HOOKER);
	VOID Patch_EagleEye(HOOKER);
	VOID Patch_Modes(HOOKER);
	VOID Patch_Subtitles(HOOKER);
	VOID Patch_Input(HOOKER);
	VOID Patch_Credits(HOOKER);
	VOID Patch_Image(HOOKER);
}