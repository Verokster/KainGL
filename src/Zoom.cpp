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
#include "Hooks.h"

#define NORMAL_SCALE 4096
#define LARGE_SCALE 4096
#define SMALL_SCALE 2432

BOOL* flagHires = (BOOL*)0x008E81E0;
DWORD* scale = (DWORD*)0x008E2EE4;

DWORD* maxScale = (DWORD*)0x008E2B98;
DWORD* minScale = (DWORD*)0x008E2B8C;
DWORD* advScale = (DWORD*)0x008E2E50;

DWORD* roomSize = (DWORD*)0x008E2E58;
DWORD* roomClip = (DWORD*)0x008E2E44;

DWORD* shiftScale = (DWORD*)0x008E2BA0;
DWORD* shiftTile = (DWORD*)0x008E2B9C;

VOID __cdecl CheckZoom(DWORD flag)
{
	if (flag && !*scale)
		*scale = LARGE_SCALE;

	if (flag == 1)
		*advScale = *flagHires ? SMALL_SCALE : LARGE_SCALE;

	*roomSize = (*scale << *shiftTile) >> *shiftScale;
	*roomClip = (*roomSize + 240) / *roomSize + 1;
}

VOID __declspec(naked) hook_0042C8BC()
{
	__asm { JMP CheckZoom }
}

namespace Hooks
{
	VOID Patch_Zoom()
	{
		PatchDWord((DWORD)shiftTile, 5);
		PatchDWord((DWORD)shiftScale, 12);
		PatchDWord((DWORD)maxScale, SMALL_SCALE);
		PatchDWord((DWORD)minScale, LARGE_SCALE);
		PatchDWord(0x008E2B94, NORMAL_SCALE); // unknown

		flagHires = (BOOL*)((BOOL)flagHires + baseAddress);
		scale = (DWORD*)((DWORD)scale + baseAddress);

		//maxScale = (DWORD*)((DWORD)maxScale + baseAddress);
		//minScale = (DWORD*)((DWORD)minScale + baseAddress);
		advScale = (DWORD*)((DWORD)advScale + baseAddress);

		roomSize = (DWORD*)((DWORD)roomSize + baseAddress);
		roomClip = (DWORD*)((DWORD)roomClip + baseAddress);

		shiftScale = (DWORD*)((DWORD)shiftScale + baseAddress);
		shiftTile = (DWORD*)((DWORD)shiftTile + baseAddress);

		PatchHook(0x0042C8BC, hook_0042C8BC);

		// Prevent from y offset inside builing
		PatchNop(0x0043E72B, 10);
		PatchNop(0x00454B9B, 10);

		// Prevent static camera
		PatchNop(0x0044E732, 5);
	}
}