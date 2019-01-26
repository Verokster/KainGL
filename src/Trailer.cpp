/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

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

namespace Hooks
{
	const CHAR* logoList[3] = {
		"ACTL001",
		"LOGO001",
		"SK"
	};

	const CHAR* trailersList[2] = {
		"TRAILERA",
		"TRAILERB"
	};

	const CHAR* __stdcall CheckTrailer(DWORD index)
	{
		if (!index)
		{
			SeedRandom(GetTickCount());
			return trailersList[Random() % (sizeof(trailersList) / sizeof(CHAR*))];
		}
		else
			return logoList[index - 1];
	}

	DWORD back_004518C6 = 0x004518C6;
	VOID __declspec(naked) hook_004518BF()
	{
		__asm
		{
			PUSH EAX
			CALL CheckTrailer
			MOV EBX, EAX
			JMP back_004518C6
		}
	}

	DWORD back_004518FA = 0x004518FA;
	VOID __declspec(naked) hook_004518F3()
	{
		__asm
		{
			PUSH EAX
			CALL CheckTrailer
			MOV EDX, EAX
			JMP back_004518FA
		}
	}

	VOID Patch_Trailer()
	{
		// Replace trailers
		PatchHook(0x004518BF, hook_004518BF);
		back_004518C6 += baseOffset;

		PatchHook(0x004518F3, hook_004518F3);
		back_004518FA += baseOffset;

		if (configVideoSkipIntro)
			PatchByte(0x0043DAA3, 0xEB);
		else
			PatchCall(0x0043DAAE, (VOID*)(0x004518DC + baseOffset));
	}
}