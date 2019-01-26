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
#include "Main.h"
#include "Config.h"

BOOL isIma;
INT imaIndex[2];
INT imaValue[2];

// Check file
DWORD* samplesRate = (DWORD*)0x004BF1EC;
WAVEFORMATEX* waveFormat = (WAVEFORMATEX*)0x004BD8C8;
VOID __cdecl CheckAudioFile(CHAR* dest, const CHAR* format, CHAR* path, CHAR* name)
{
	BOOL found = FALSE;
	StrPrint(dest, "GAME/%s.IMA", name);
	DWORD hash = ((DWORD(__cdecl*)(CHAR*))Hooks::sub_GetHash)(dest);
	FileHeader* file = filesHeaders;
	DWORD count = *filesCount;
	while (count--)
	{
		if (file->hash == hash)
		{
			found = TRUE;
			break;
		}

		++file;
	}

	if (found)
	{
		isIma = TRUE;
		*samplesRate = 37800; // sample rate 
		waveFormat->nChannels = 2; // nChannels
		imaIndex[1] = imaIndex[0] = 0;
		imaValue[1] = imaValue[0] = 0;
	}
	else
	{
		StrPrint(dest, format, path, name);

		isIma = FALSE;
		waveFormat->nChannels = 1; // nChannels
	}

	waveFormat->nSamplesPerSec = *samplesRate;
	waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
	waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec * waveFormat->nBlockAlign;
}

DWORD back_00451A6F = 0x00451A6F;
DWORD some_008E81E0 = 0x008E81E0;
VOID __declspec(naked) hook_00451A69()
{
	__asm
	{
		XOR ECX, ECX
		MOV isIma, ECX
		MOV ECX, some_008E81E0
		MOV ECX, DWORD PTR DS : [ECX]
		JMP back_00451A6F
	}
}

INT* indexTable = (INT*)0x004BD8DC;
INT* stepsizeTable = (INT*)0x004BD91C;
BYTE* lastIndex = (BYTE*)0x004BDA82;
SHORT* lastValue = (SHORT*)0x004BDA80;
VOID __cdecl Convert4bitTo16bit(BYTE* input, SHORT* output, INT count)
{
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

DWORD back_0044484E = 0x0044484E;
VOID __declspec(naked) hook_00444846()
{
	__asm
	{
		MOV EAX, isIma
		TEST EAX, EAX
		JZ lbl_VAG
		PUSH 1
		JMP lbl_cont
		lbl_VAG : PUSH 3
				  lbl_cont : PUSH 0
							 MOV EDX, DWORD PTR SS : [ESP + 0x18]
							 JMP back_0044484E
	}
}

DWORD back_00451A09 = 0x00451A09;
DWORD back_00451AF6 = 0x00451AF6;
VOID __declspec(naked) hook_00451A01()
{
	__asm
	{
		MOV ECX, isIma
		TEST ECX, ECX
		JZ lbl_VAG
		JMP back_00451A09

		lbl_VAG : TEST EAX, EAX
				  JZ lbl_notfound
				  JMP back_00451A09

				  lbl_notfound : JMP back_00451AF6
	}
}

// =====================================================
#pragma region "Load Flash audio"
DWORD sub_004453E8 = 0x004453E8;
VOID LoadFlashAudio()
{
	CHAR fileName[10];
	for (DWORD i = 0; i < 4; ++i)
	{
		StrPrint(fileName, "THN%d.VAG", i + 1);
		((VOID(__cdecl*)(CHAR*, DWORD))sub_004453E8)(fileName, 44 + i); // WaveStopAndRealocate
	}
}

DWORD back_00445561 = 0x00445561;
VOID __declspec(naked) hook_00445530()
{
	__asm
	{
		CALL LoadFlashAudio
		JMP back_00445561
	}
}
#pragma endregion

// =====================================================
BOOL* isCameraStatic = (BOOL*)0x005947CC;
DWORD* worldObject = (DWORD*)0x004BF7CC;

#pragma region "Check listener position"
BYTE* offsetListX = (BYTE*)0x00414948;
BYTE* offsetListY = (BYTE*)0x00414952;
VOID CheckListenePosition()
{
	if (!dsoundList)
		return;

	INT offset = (INT)MathRound(120.0f * 4096 / *scale);

	INT checkX = 0;
	INT checkY = 0;
	if (!*isCameraStatic)
	{
		INT kainDirection = *((INT*)*worldObject + 10);
		INT check = *(SHORT*)*worldObject;
		if (check <= 0)
			kainDirection = DIR_NONE;

		checkX = 120 - *(offsetListX + kainDirection);
		checkY = 120 - *(offsetListY + kainDirection);
	}

	INT kainX = *((WORD*)*worldObject + 9);
	INT kainY = *((WORD*)*worldObject + 11);

	INT camX = kainX + checkX;
	if (camX < offset)
		camX = offset;
	else if (camX > 2560 - offset)
		camX = 2560 - offset;

	INT camY = kainY + checkY;
	if (camY < offset)
		camY = offset;
	else if (camY > 2560 - offset)
		camY = 2560 - offset;

	FLOAT point[3] = {
		(FLOAT)camX,
		(FLOAT)camY,
		320.0f * 4096 / *scale };

	dsoundList->SetListener(point);
}

DWORD back_00416A6E = 0x00416A6E;
VOID __declspec(naked) hook_00416A68()
{
	__asm
	{
		CALL CheckListenePosition

		PUSH EBX
		PUSH ESI
		PUSH EDI
		SUB ESP, 0x18

		JMP back_00416A6E
	}
}

DWORD back_0041684F = 0x0041684F;
VOID __declspec(naked) hook_00416848()
{
	__asm
	{
		CALL CheckListenePosition

		PUSH EBX
		PUSH ESI
		PUSH EDI
		PUSH EBP
		SUB ESP, 0x20

		JMP back_0041684F
	}
}
#pragma endregion

// =====================================================
INT* gainVolume = (INT*)0x004BFA48;
bool* isKainInside = (bool*)0x008BF35D;
BOOL* isKainSpeaking = (BOOL*)0x004BFFD8;

#pragma region "3D sound"
BOOL isGainPositional;
ALSoundOptions soundOptions;

DWORD objectCellX, objectCellY;

DWORD back_00445D60 = 0x00445D60;
VOID __declspec(naked) hook_00445D58()
{
	__asm
	{
		MOV EAX, [ESP + 0x0C]
		MOV objectCellX, EAX
		MOV EAX, [ESP + 0x10]
		MOV objectCellY, EAX
		XOR EAX, EAX
		INC EAX
		MOV isGainPositional, EAX

		PUSH EBX
		PUSH ESI
		PUSH EDI
		PUSH EBP
		MOV EDI, DWORD PTR SS : [ESP + 0x14]

		JMP back_00445D60
	}
}

VOID __stdcall CheckPositionalSound(DWORD* waveIndex)
{
	soundOptions.isPositional = TRUE;
	soundOptions.waveIndex = *waveIndex;

	switch (*waveIndex)
	{
	case SND_CLICK:
	case SND_HEARTB:
	case SND_INV:
		break;

	case SND_HOWL05:
	{
		soundOptions.isAbsolute = FALSE;
		soundOptions.rolloffFactor = 0.0f;
		soundOptions.referenceDistance = 240.0f;

		FLOAT radios = *isKainInside ? 120.0f : 240.0f;
		soundOptions.position.x = (1.0f - (FLOAT)Random() / (RAND_MAX >> 1)) * radios;
		soundOptions.position.y = (1.0f - (FLOAT)Random() / (RAND_MAX >> 1)) * radios;
		soundOptions.position.z = 0.0f;
		break;
	}

	case SND_FLASH:
	{
		*waveIndex += Random() % 3;
		soundOptions.waveIndex = *waveIndex;

		soundOptions.isAbsolute = FALSE;
		soundOptions.rolloffFactor = 0.0f;
		soundOptions.referenceDistance = 240.0f;

		FLOAT radios = *isKainInside ? 120.0f : 240.0f;
		soundOptions.position.x = (1.0f - (FLOAT)Random() / (RAND_MAX >> 1)) * radios;
		soundOptions.position.y = (1.0f - (FLOAT)Random() / (RAND_MAX >> 1)) * radios;
		soundOptions.position.z = 120.0f;
		break;
	}

	default:
	{
		soundOptions.isAbsolute = TRUE;
		soundOptions.rolloffFactor = 6.0f;
		soundOptions.referenceDistance = 500.0f;

		if (isGainPositional)
		{
			soundOptions.position.x = FLOAT(objectCellX * 32 + 16);
			soundOptions.position.y = FLOAT(objectCellY * 32 + 16);
		}
		else
		{
			soundOptions.position.x = (FLOAT)*((WORD*)*worldObject + 9);
			soundOptions.position.y = (FLOAT)*((WORD*)*worldObject + 11);
		}

		soundOptions.position.z = 0.0f;
		break;
	}
	}

	isGainPositional = FALSE;
}

DWORD back_00445348 = 0x00445348;
VOID __declspec(naked) hook_00445340()
{
	__asm
	{
		LEA EAX, [ESP + 0x4]
		PUSH EAX
		CALL CheckPositionalSound

		PUSH EBX
		PUSH ESI
		PUSH EDI
		PUSH EBP
		MOV EBX, DWORD PTR SS : [ESP + 0x1C]

		JMP back_00445348
	}
}
#pragma endregion

namespace Hooks
{
	VOID Patch_Audio()
	{
		isCameraStatic = (BOOL*)((DWORD)isCameraStatic + baseOffset);

		gainVolume = (INT*)((DWORD)gainVolume + baseOffset);
		isKainInside = (bool*)((DWORD)isKainInside + baseOffset);
		isKainSpeaking = (BOOL*)((DWORD)isKainSpeaking + baseOffset);

		worldObject = (DWORD*)((DWORD)worldObject + baseOffset);

		samplesRate = (DWORD*)((DWORD)samplesRate + baseOffset);
		waveFormat = (WAVEFORMATEX*)((DWORD)waveFormat + baseOffset);
		sub_004453E8 += baseOffset;

		indexTable = (INT*)((DWORD)indexTable + baseOffset);
		stepsizeTable = (INT*)((DWORD)stepsizeTable + baseOffset);
		lastIndex = (BYTE*)((DWORD)lastIndex + baseOffset);
		lastValue = (SHORT*)((DWORD)lastValue + baseOffset);

		offsetListX = (BYTE*)((DWORD)offsetListX + baseOffset);
		offsetListY = (BYTE*)((DWORD)offsetListY + baseOffset);

		PatchCall(0x004519C4, CheckAudioFile); // check file

		PatchHook(0x00451A69, hook_00451A69); // restore
		back_00451A6F += baseOffset;
		some_008E81E0 += baseOffset;

		PatchNop(0x0044778E, 0x0044779A - 0x0044778E); // 
		PatchHook(0x00447064, hook_00447064);

		PatchHook(0x00451A01, hook_00451A01); // Do not check if audio file for video exists
		back_00451A09 += baseOffset;
		back_00451AF6 += baseOffset;

		PatchHook(0x00444846, hook_00444846);
		back_0044484E += baseOffset;

		// Increase wave pool 46 -> 49
		PatchDWord(0x004453BC + 1, 49 * 4);
		PatchNop(0x004453CF, 6);

		// Move inventory index 45 -> 48
		DWORD val = 0x005915EC + 48 * 4 + baseOffset;
		PatchDWord(0x00446988 + 2, val);
		PatchDWord(0x004469AF + 1, val);
		PatchDWord(0x004458C0 + 2, val);
		PatchDWord(0x0044695B + 2, val);
		PatchDWord(0x004469DF + 2, val);

		PatchHook(0x00445530, hook_00445530); // Load Flash audio
		back_00445561 += baseOffset;

		// 3d audio
		PatchHook(0x00445D58, hook_00445D58); // Get sound cell position
		back_00445D60 += baseOffset;

		PatchHook(0x00445340, hook_00445340); // Check sound index for positioning
		back_00445348 += baseOffset;

		if (configOther3DSound) // remove max distance
			PatchByte(0x00445DBB, 0xEB);

		// Check listener position
		PatchHook(0x00416A68, hook_00416A68);
		back_00416A6E += baseOffset;

		PatchHook(0x00416848, hook_00416848);
		back_0041684F += baseOffset;

		// Prevent "Coffee Guy"
		PatchJump(0x00419C5F, 0x00419C92 + baseOffset);
	}
}