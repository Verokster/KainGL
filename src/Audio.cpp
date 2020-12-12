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
#include "Main.h"
#include "Config.h"

BOOL isIma;
BOOL foundIma;
INT imaIndex[2];
INT imaValue[2];

// Check file
WAVEFORMATEX* waveFormat;
VOID __cdecl CheckAudioFile(CHAR* dest, const CHAR* format, CHAR* path, CHAR* name)
{
	foundIma = FALSE;
	StrPrint(dest, "GAME/%s.IMA", name);
	DWORD hash = ((DWORD(__cdecl*)(CHAR*))Hooks::sub_GetHash)(dest);
	FileHeader* file = filesHeaders;
	DWORD count = *filesCount;
	while (count--)
	{
		if (file->hash == hash)
		{
			foundIma = TRUE;
			return;
		}

		++file;
	}

	StrPrint(dest, format, path, name);
}

DWORD sub_CreateVideoSoundBuffer;
VOID __cdecl CreateVideoSoundBuffer(DWORD rate)
{
	isIma = foundIma;
	if (foundIma)
	{
		rate = 37800;
		waveFormat->nChannels = 2;
		imaIndex[1] = imaIndex[0] = 0;
		imaValue[1] = imaValue[0] = 0;
	}
	else
		waveFormat->nChannels = 1;

	waveFormat->nSamplesPerSec = rate;
	waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
	waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec * waveFormat->nBlockAlign;

	((DWORD(__cdecl*)(DWORD))sub_CreateVideoSoundBuffer)(rate);
}

DWORD sub_CreateRegularSoundBuffer;
VOID __cdecl CreateRegularSoundBuffer()
{
	((DWORD(__cdecl*)())sub_CreateRegularSoundBuffer)();
	isIma = FALSE;
}

INT* indexTable;
INT* stepsizeTable;
BYTE* lastIndex;
SHORT* lastValue;
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
	__asm { jmp Convert4bitTo16bit }
}

DWORD back_0044484E;
VOID __declspec(naked) hook_00444846()
{
	__asm {
		mov eax, isIma
		test eax, eax
		jz lbl_VAG
		push 1
		jmp lbl_cont
		lbl_VAG : push 3
				  lbl_cont : push 0
							 mov edx, [esp + 0x18]
							 jmp back_0044484E
	}
}

DWORD back_00451A09;
DWORD back_00451AF6;
VOID __declspec(naked) hook_00451A01()
{
	__asm {
		mov ecx, foundIma
		test ecx, ecx
		jz lbl_VAG
		jmp back_00451A09

		lbl_VAG : test eax, eax
				  jz lbl_notfound
				  jmp back_00451A09

				  lbl_notfound : jmp back_00451AF6
	}
}

// =====================================================
#pragma region "Load Flash audio"
DWORD sub_004453E8;
VOID LoadFlashAudio()
{
	CHAR fileName[10];
	for (DWORD i = 0; i < 4; ++i)
	{
		StrPrint(fileName, "THN%d.VAG", i + 1);
		((VOID(__cdecl*)(CHAR*, DWORD))sub_004453E8)(fileName, 44 + i); // WaveStopAndRealocate
	}
}

DWORD back_00445561;
VOID __declspec(naked) hook_00445530()
{
	__asm
	{
		call LoadFlashAudio
		jmp back_00445561
	}
}
#pragma endregion

// =====================================================
BOOL* isCameraStatic;
DWORD* worldObject;

#pragma region "Check listener position"
BYTE* offsetListX;
BYTE* offsetListY;
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
		320.0f * 4096 / *scale
	};

	dsoundList->SetListener(point);
}

DWORD back_00416A6E;
VOID __declspec(naked) hook_00416A68()
{
	__asm
	{
		call CheckListenePosition

		push ebx
		push esi
		push edi
		sub esp, 0x18

		jmp back_00416A6E
	}
}

DWORD back_0041684F;
VOID __declspec(naked) hook_00416848()
{
	__asm
	{
		call CheckListenePosition

		push ebx
		push esi
		push edi
		push ebp
		sub esp, 0x20

		jmp back_0041684F
	}
}
#pragma endregion

// =====================================================
INT* gainVolume;
bool* isKainInside;
BOOL* isKainSpeaking;

#pragma region "3D sound"
BOOL isGainPositional;
ALSoundOptions soundOptions;

DWORD objectCellX, objectCellY;

DWORD back_00445D60;
VOID __declspec(naked) hook_00445D58()
{
	__asm
	{
		mov eax, [esp + 0xC]
		mov objectCellX, eax
		mov eax, [esp + 0x10]
		MOV objectCellY, eax
		xor eax, eax
		inc eax
		mov isGainPositional, eax

		push ebx
		push esi
		push edi
		push ebp
		mov edi, [esp + 0x14]

		jmp back_00445D60
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
			soundOptions.position.x = (FLOAT) * ((WORD*)*worldObject + 9);
			soundOptions.position.y = (FLOAT) * ((WORD*)*worldObject + 11);
		}

		soundOptions.position.z = 0.0f;
		break;
	}
	}

	isGainPositional = FALSE;
}

DWORD back_00445348;
VOID __declspec(naked) hook_00445340()
{
	__asm
	{
		lea eax, [esp + 0x4]
		push eax
		call CheckPositionalSound

		push ebx
		push esi
		push edi
		push ebp
		mov ebx, [esp + 0x1c]

		jmp back_00445348
	}
}
#pragma endregion

namespace Hooks
{
#pragma optimize("s", on)
	VOID Patch_Audio(HOOKER hooker)
	{
		DWORD baseOffset = GetBaseOffset(hooker);

		isCameraStatic = (BOOL*)f(0x005947CC);

		gainVolume = (INT*)f(0x004BFA48);
		isKainInside = (bool*)f(0x008BF35D);
		isKainSpeaking = (BOOL*)f(0x004BFFD8);

		worldObject = (DWORD*)f(0x004BF7CC);

		waveFormat = (WAVEFORMATEX*)f(0x004BD8C8);
		sub_004453E8 = f(0x004453E8);

		indexTable = (INT*)f(0x004BD8DC);
		stepsizeTable = (INT*)f(0x004BD91C);
		lastIndex = (BYTE*)f(0x004BDA82);
		lastValue = (SHORT*)f(0x004BDA80);

		offsetListX = (BYTE*)f(0x00414948);
		offsetListY = (BYTE*)f(0x00414952);

		PatchCall(hooker, 0x004519C4, CheckAudioFile); // check file

		// init
		sub_CreateVideoSoundBuffer = RedirectCall(hooker, 0x00451A28, CreateVideoSoundBuffer);

		// restore
		sub_CreateRegularSoundBuffer = RedirectCall(hooker, 0x00451A64, CreateRegularSoundBuffer);

		// prevent wave format calculation
		PatchJump(hooker, 0x0044778E, f(0x0044779A));
		PatchHook(hooker, 0x00447064, hook_00447064);

		PatchHook(hooker, 0x00451A01, hook_00451A01); // Do not check if audio file for video exists
		back_00451A09 = f(0x00451A09);
		back_00451AF6 = f(0x00451AF6);

		PatchHook(hooker, 0x00444846, hook_00444846);
		back_0044484E = f(0x0044484E);

		// Increase wave pool 46 -> 49
		PatchDWord(hooker, 0x004453BC + 1, 49 * 4);
		PatchNop(hooker, 0x004453CF, 6);

		// Move inventory index 45 -> 48
		DWORD val = f(0x005915EC) + 48 * 4;
		PatchDWord(hooker, 0x00446988 + 2, val);
		PatchDWord(hooker, 0x004469AF + 1, val);
		PatchDWord(hooker, 0x004458C0 + 2, val);
		PatchDWord(hooker, 0x0044695B + 2, val);
		PatchDWord(hooker, 0x004469DF + 2, val);

		PatchHook(hooker, 0x00445530, hook_00445530); // Load Flash audio
		back_00445561 = f(0x00445561);

		// 3d audio
		PatchHook(hooker, 0x00445D58, hook_00445D58); // Get sound cell position
		back_00445D60 = f(0x00445D60);

		PatchHook(hooker, 0x00445340, hook_00445340); // Check sound index for positioning
		back_00445348 = f(0x00445348);

		if (config.other.sound3d) // remove max distance
			PatchByte(hooker, 0x00445DBB, 0xEB);

		// Check listener position
		PatchHook(hooker, 0x00416A68, hook_00416A68);
		back_00416A6E = f(0x00416A6E);

		PatchHook(hooker, 0x00416848, hook_00416848);
		back_0041684F = f(0x0041684F);

		// Prevent "Coffee Guy"
		PatchJump(hooker, 0x00419C5F, f(0x00419C92));
	}
#pragma optimize("", on)
}