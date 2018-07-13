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

BOOL isIma;
INT imaIndex[2];
INT imaValue[2];

// Check file
VOID __stdcall CheckFile(CHAR* dest, const CHAR* format, CHAR* path, CHAR* name)
{
	DWORD* samplesRate = (DWORD*)0x004BF1EC;
	WORD* nChannels = (WORD*)0x004BD8CA;
	DWORD* nSamplesPerSec = (DWORD*)0x004BD8CC;
	DWORD* nAvgBytesPerSec = (DWORD*)0x004BD8D0;
	WORD* nBlockAlign = (WORD*)0x004BD8D4;
	WORD* wBitsPerSample = (WORD*)0x004BD8D6;

	StrPrint(dest, "%s\\%s.IMA", path, name);
	FILE* file = FileOpen(dest, "r");
	if (file)
	{
		FileClose(file);
		isIma = TRUE;
		*samplesRate = 37800; // sample rate 
		*nChannels = 2; // nChannels
		imaIndex[1] = imaIndex[0] = 0;
		imaValue[1] = imaValue[0] = 0;
	}
	else
	{
		StrPrint(dest, format, path, name);
		isIma = FALSE;
		*nChannels = 1; // nChannels
	}

	*nSamplesPerSec = *samplesRate;
	*nBlockAlign = *nChannels * *wBitsPerSample / 8;
	*nAvgBytesPerSec = *nSamplesPerSec * *nBlockAlign;
}

DWORD back_004519CC = 0x004519CC;
VOID __declspec(naked) hook_004519C4()
{
	__asm
	{
		CALL CheckFile
		JMP back_004519CC
	}
}

DWORD back_00451A6F = 0x00451A6F;
VOID __declspec(naked) hook_00451A69()
{
	__asm
	{
		XOR ECX, ECX
		MOV isIma, ECX
		MOV ECX, DWORD PTR DS : [0x008E81E0]
		JMP back_00451A6F
	}
}

VOID __cdecl Convert4bitTo16bit(BYTE* input, SHORT* output, INT count)
{
	INT* indexTable = (INT*)0x004BD8DC;
	INT* stepsizeTable = (INT*)0x004BD91C;
	
	BOOL isRight = FALSE;
	INT delta;

	if (isIma)
	{
		INT imaStep[2] = { stepsizeTable[imaIndex[0]], stepsizeTable[imaIndex[1]] };

		BOOL isSecond = FALSE;

		++input;
		while (count--)
		{
			INT* index = &imaIndex[isRight];
			INT* value = &imaValue[isRight];
			INT* step = &imaStep[isRight];

			if (!isRight)
			{
				delta = *input & 0xF;
			}
			else
			{
				delta = *input >> 4;

				if (!isSecond)
					--input;
				else
					input += 3;

				isSecond = !isSecond;
			}
			
			isRight = !isRight;

			*index += indexTable[delta];

			if (*index < 0)
				*index = 0;

			if (*index > 88)
				*index = 88;

			INT vpdiff = *step >> 3;

			if (delta & 4)
				vpdiff += *step;

			if (delta & 2)
				vpdiff += *step >> 1;

			if (delta & 1)
				vpdiff += *step >> 2;

			if (delta & 8)
				*value -= vpdiff;
			else
				*value += vpdiff;

			if (*value > 32767)
				*value = 32767;
			else if (*value < -32768)
				*value = -32768;

			*step = stepsizeTable[*index];
			*output++ = *value;
		}
	}
	else
	{
		BYTE* lastIndex = (BYTE*)0x004BDA82;
		SHORT* lastValue = (SHORT*)0x004BDA80;

		INT index = *lastIndex;
		INT value = *lastValue;
		INT step = stepsizeTable[index];

		while (count--)
		{
			delta = isRight ? *input++ & 0xF : *input >> 4;
			isRight = !isRight;

			index += indexTable[delta];

			if (index < 0)
				index = 0;

			if (index > 88)
				index = 88;

			INT vpdiff = step >> 3;

			if (delta & 4)
				vpdiff += step;

			if (delta & 2)
				vpdiff += step >> 1;

			if (delta & 1)
				vpdiff += step >> 2;

			if (delta & 8)
				value -= vpdiff;
			else
				value += vpdiff;

			if (value > 32767)
				value = 32767;
			else if (value < -32768)
				value = -32768;

			step = stepsizeTable[index];
			*output++ = value;
		}

		*lastIndex = index;
		*lastValue = value;
	}
}

VOID __declspec(naked) hook_00447064()
{
	__asm { JMP Convert4bitTo16bit }
}

namespace Hooks
{
	VOID Patch_Audio()
	{
		PatchHook(0x004519C4, hook_004519C4); // check file
		PatchHook(0x00451A69, hook_00451A69); // restore
		PatchNop(0x0044778E, 0x0044779A - 0x0044778E); // 
		PatchHook(0x00447064, hook_00447064);
	}
}