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

#include "stdafx.h"
#include "Hooks.h"
#include "Config.h"

#define NORMAL_SCALE 4096
#define LARGE_SCALE 4096
#define SMALL_SCALE 2432

BOOL* flagHires;
DWORD* scale;

DWORD* advScale;

DWORD* roomSize;
DWORD* roomClip;

DWORD* shiftScale;
DWORD* shiftTile;

VOID __cdecl CheckZoom(DWORD flag)
{
	if (flag && !*scale)
		*scale = !config.camera.isZoomed ? SMALL_SCALE : LARGE_SCALE;

	if (flag == 1)
		*advScale = *flagHires ? SMALL_SCALE : LARGE_SCALE;

	*roomSize = (*scale << *shiftTile) >> *shiftScale;
	*roomClip = (*roomSize + 240) / *roomSize + 1;
}

namespace Hooks
{
#pragma optimize("s", on)
	VOID Patch_Zoom(HOOKER hooker)
	{
		DWORD baseOffset = GetBaseOffset(hooker);

		PatchDWord(hooker, 0x008E2B98, SMALL_SCALE); // max scale
		PatchDWord(hooker, 0x008E2B8C, LARGE_SCALE); // min scale
		PatchDWord(hooker, 0x008E2B94, NORMAL_SCALE); // unknown

		PatchDWord(hooker, 0x008E2B9C, 5);
		PatchDWord(hooker, 0x008E2BA0, 12);

		shiftTile = (DWORD*)f(0x008E2B9C);
		shiftScale = (DWORD*)f(0x008E2BA0);

		flagHires = (BOOL*)f(0x008E81E0);
		scale = (DWORD*)f(0x008E2EE4);

		advScale = (DWORD*)f(0x008E2E50);

		roomSize = (DWORD*)f(0x008E2E58);
		roomClip = (DWORD*)f(0x008E2E44);

		PatchHook(hooker, 0x0042C8BC, CheckZoom);

		// Prevent from y offset inside builing
		PatchNop(hooker, 0x0043E72B, 10);
		PatchNop(hooker, 0x00454B9B, 10);

		// Prevent static camera
		PatchNop(hooker, 0x0044E732, 5);
	}
#pragma optimize("", on)
}