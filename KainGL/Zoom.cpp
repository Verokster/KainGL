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

VOID __stdcall CheckZoom(DWORD flag)
{
	if (flag)
	{
		if (*flagHires)
		{
			*scale = SMALL_SCALE;
			*(DWORD*)0x008E2E50 = LARGE_SCALE;

			*(DWORD*)0x008E2B98 = SMALL_SCALE;
			*(DWORD*)0x008E2B8C = LARGE_SCALE;
		}
		else
		{
			if (flag == 1)
			{
				*scale = LARGE_SCALE;
				*(DWORD*)0x008E2E50 = LARGE_SCALE;
			}

			*(DWORD*)0x008E2B98 = SMALL_SCALE;
			*(DWORD*)0x008E2B8C = LARGE_SCALE;
		}
	}

	*(DWORD*)0x008E2B94 = NORMAL_SCALE;

	*(DWORD*)0x008E2BA0 = 12;
	*(DWORD*)0x008E2B9C = 5;

	*(DWORD*)0x008E2E58 = 32 * *scale / NORMAL_SCALE;
	*(DWORD*)0x008E2E44 = (*(DWORD*)0x008E2E58 + 241) / *(DWORD*)0x008E2E58 + 1;
}

VOID __declspec(naked) hook_42C8BC()
{
	__asm
	{
		MOV EAX, [ESP + 4]
		PUSH EAX
		CALL CheckZoom
		RETN
	}
}

namespace Hooks
{
	VOID Patch_Zoom()
	{
		PatchHook(0x42C8BC, hook_42C8BC);
	}
}