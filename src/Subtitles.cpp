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
#include "Config.h"

IDirectSoundBuffer* activeSoundBuffer;
DWORD soundStartTime;
DWORD soundSuspendTime;
BOOL soundIsInventory;

SubtitlesItem* subtitlesList;
SubtitlesItem* subtitlesCurrent;
DWORD subtitlesCount;

VOID __stdcall CheckPlay(CHAR* fileName)
{
	CHAR* ptr = StrRightChar(fileName, '/');
	if (!ptr)
		ptr = StrRightChar(fileName, '\\');

	if (ptr)
		++ptr;
	else
		ptr = fileName;

	CHAR soundFileName[16];
	StrCopy(soundFileName, ptr);

	ptr = StrRightChar(soundFileName, '.');
	if (ptr)
		*ptr = NULL;

	BOOL found = FALSE;
	SubtitlesItem* current = subtitlesList;
	DWORD count = subtitlesCount;
	while (count--)
	{
		if (!StrCompare(soundFileName, current->key))
		{
			subtitlesCurrent = current;
			subtitlesCurrent->flags = soundIsInventory;
			found = TRUE;
			break;
		}

		++current;
	}

	soundIsInventory = FALSE;

	if (found)
	{
		soundStartTime = GetTickCount();
		soundSuspendTime = 0;
	}
	else
		subtitlesCurrent = NULL;
}

DWORD sub_00448834 = 0x00448834;
DWORD back_00447836 = 0x00447836;
VOID __declspec(naked) hook_00447831()
{
	__asm
	{
		PUSH EBP
		CALL CheckPlay

		CALL sub_00448834
		JMP back_00447836
	}
}

DWORD sub_00446988 = 0x00446988;
DWORD back_00445895 = 0x00445895;
VOID __declspec(naked) hook_00445890()
{
	__asm
	{
		PUSH EAX
		CALL CheckPlay

		CALL sub_00446988
		JMP back_00445895
	}
}

DWORD back_00458D86 = 0x00458D86;
VOID __declspec(naked) hook_00458D80()
{
	__asm
	{
		XOR EAX, EAX
		MOV subtitlesCurrent, EAX

		PUSH    EBX
		PUSH    ESI
		PUSH    EDI
		PUSH    EBP
		XOR     AH, AH

		JMP back_00458D86
	}
}

DWORD back_0045ABD9 = 0x0045ABD9;
DWORD sub_00438B70 = 0x00438B70; // CheckAndPlay
DWORD some_004C0A28 = 0x004C0A28;
VOID __declspec(naked) hook_0045ABD4()
{
	__asm
	{
		MOV EAX, some_004C0A28
		MOV EAX, DWORD PTR DS : [EAX]
		TEST EAX, EAX
		JZ lbl_back
		CALL sub_00438B70
		lbl_back : JMP back_0045ABD9
	}
}

DWORD sub_004386F0 = 0x004386F0;
DWORD back_004458F3 = 0x004458F3;
VOID __declspec(naked) hook_004458EE()
{
	__asm
	{
		XOR EAX, EAX
		INC EAX
		MOV soundIsInventory, EAX

		CALL sub_004386F0
		JMP back_004458F3
	}
}

DWORD back_004409A3 = 0x004409A3;
VOID __declspec(naked) hook_0044099E()
{
	__asm
	{
		XOR EAX, EAX
		INC EAX
		MOV soundIsInventory, EAX

		CALL sub_004386F0
		JMP back_004409A3
	}
}

// 0044099E

namespace Hooks
{
	VOID Patch_Subtitles()
	{
		if (configLangSubtitles < 0)
			return;

		// Check for subtitles
		PatchHook(0x00447831, hook_00447831);
		sub_00448834 += baseAddress;
		back_00447836 += baseAddress;

		PatchHook(0x00445890, hook_00445890);
		sub_00446988 += baseAddress;
		back_00445895 += baseAddress;

		// Option music - release subtitles
		PatchHook(0x00458D80, hook_00458D80);
		back_00458D86 += baseAddress;

		PatchHook(0x0045ABD4, hook_0045ABD4);
		back_0045ABD9 += baseAddress;
		sub_00438B70 += baseAddress;
		some_004C0A28 += baseAddress;

		// Set flags for display rectangle
		sub_004386F0 += baseAddress;

		PatchHook(0x004458EE, hook_004458EE);
		back_004458F3 += baseAddress;

		PatchHook(0x0044099E, hook_0044099E);
		back_004409A3 += baseAddress;

		FILE* hFile = FileOpen(langFiles.subtitlesFile, "rb");
		if (hFile)
		{
			VOID* fileBuffer = MemoryAlloc(256 * 1024);
			{
				DWORD count = FileRead(fileBuffer, 256 * 1024, 1, hFile);
				BYTE* ptr = (BYTE*)fileBuffer;

				if (*(DWORD*)ptr == 0x00425553)
				{
					ptr += sizeof(DWORD);

					DWORD keysCount = subtitlesCount = *(WORD*)ptr;
					ptr += sizeof(WORD);

					subtitlesList = (SubtitlesItem*)MemoryAlloc(sizeof(SubtitlesItem) * keysCount);

					SubtitlesItem* subItem = subtitlesList;
					while (keysCount--)
					{
						DWORD length = StrLength((CHAR*)ptr);
						subItem->key = (CHAR*)MemoryAlloc(length + 1);
						MemoryCopy(subItem->key, ptr, length + 1);
						ptr += length + 1;

						if (StrStr(subItem->key, "ACT") == subItem->key)
							subItem->type = SubtitlesVideo;
						else if (StrStr(subItem->key, "AV") == subItem->key)
							subItem->type = SubtitlesWalk;
						else if (StrStr(subItem->key, "DES") == subItem->key)
							subItem->type = SubtitlesDescription;
						else if (StrStr(subItem->key, "INV") == subItem->key)
							subItem->type = SubtitlesInventory;
						else
							subItem->type = SubtitlesOther;

						DWORD linesCount = subItem->count = *(WORD*)ptr;
						ptr += sizeof(WORD);

						subItem->lines = (SubtitlesLine*)MemoryAlloc(sizeof(SubtitlesLine) * linesCount);

						SubtitlesLine* prevLine = NULL;
						SubtitlesLine* subLine = subItem->lines;
						while (linesCount--)
						{
							subLine->startTick = *(DWORD*)ptr;
							ptr += sizeof(DWORD);

							subLine->endTick = *(DWORD*)ptr;
							ptr += sizeof(DWORD);

							DWORD length = 0;
							BYTE* ch = ptr;
							while (*ch)
							{
								if (!(*ch & 0x80) || (*ch & 0xC0) == 0xC0)
									++length;

								++ch;
							}

							subLine->text = (WCHAR*)MemoryAlloc((length << 1) + sizeof(WORD));
							*(subLine->text + length) = 0;

							DWORD code = 0;
							length = 0;
							ch = ptr;
							WORD* str = (WORD*)subLine->text;
							while (*ch)
							{
								DWORD check = 0x80;
								if (*ch & check)
								{
									check >>= 1;
									if (*ch & check)
									{
										if (length)
											*str++ = LOWORD(code);

										INT mask = 0xFFFFFFC0;
										do
										{
											check >>= 1;
											mask >>= 1;
										} while (*ch & check);

										code = *ch & (~mask);
									}
									else
										code = (code << 6) | (*ch & 0x3F);
								}
								else
								{
									if (length)
										*str++ = LOWORD(code);

									code = *ch;
								}

								++length;
								++ch;
							}
							if (length)
								*str++ = LOWORD(code);

							ptr += length + 1;

							if (prevLine)
							{
								if (prevLine->endTick && !(prevLine->endTick & 0x80000000))
									prevLine->endTick += prevLine->startTick - 1;
								else
									prevLine->endTick = subLine->startTick + *(INT*)&prevLine->endTick - 1;
							}

							prevLine = subLine++;
						}

						if (prevLine)
						{
							if (prevLine->endTick && !(prevLine->endTick & 0x80000000))
								prevLine->endTick += prevLine->startTick - 1;
							else
								prevLine->endTick = 0xFFFFFFFF;
						}

						++subItem;
					}
				}
			}
			MemoryFree(fileBuffer);
			FileClose(hFile);
		}
	}
}