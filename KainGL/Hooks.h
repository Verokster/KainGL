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

namespace Hooks
{
	VOID __fastcall PatchHook(DWORD addr, VOID* hook);
	VOID __fastcall PatchNop(DWORD addr, DWORD size);
	VOID __fastcall PatchWord(DWORD addr, WORD value);
	VOID __fastcall PatchInt(DWORD addr, INT value);
	VOID __fastcall PatchDWord(DWORD addr, DWORD value);
	VOID __fastcall PatchByte(DWORD addr, BYTE value);
	DWORD __fastcall ReadDWord(DWORD addr);

	BOOL Load();

	VOID Patch_Library();
	VOID Patch_System();
	VOID Patch_Window();
	VOID Patch_Video();
	VOID Patch_Mouse();
	VOID Patch_Movie();
	VOID Patch_NoCD();
	VOID Patch_Language();
}