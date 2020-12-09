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
#include "Main.h"

IDirectSoundBuffer* activeSoundBuffer;
DWORD soundStartTime;
DWORD soundSuspendTime;
BOOL soundIsInventory;

SubtitlesItem* subtitlesList;
SubtitlesItem* subtitlesCurrent;
DWORD subtitlesCount;
SubtitlesFont subtitlesFonts[2];
CHAR fontFiles[2][MAX_PATH];

VOID __stdcall CheckPlay(CHAR* fileName)
{
	CHAR* ptr = StrLastChar(fileName, '/');
	if (!ptr)
		ptr = StrLastChar(fileName, '\\');

	if (ptr)
		++ptr;
	else
		ptr = fileName;

	CHAR soundFileName[16];
	StrCopy(soundFileName, ptr);

	ptr = StrLastChar(soundFileName, '.');
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
		soundStartTime = timeGetTime();
		soundSuspendTime = 0;
	}
	else
		subtitlesCurrent = NULL;
}

DWORD sub_00448834;
DWORD back_00447836;
VOID __declspec(naked) hook_00447831()
{
	__asm
	{
		push ebp
		call CheckPlay

		call sub_00448834
		jmp back_00447836
	}
}

DWORD sub_00446988;
DWORD back_00445895;
VOID __declspec(naked) hook_00445890()
{
	__asm
	{
		push eax
		call CheckPlay

		call sub_00446988
		jmp back_00445895
	}
}

DWORD back_00458D86;
VOID __declspec(naked) hook_00458D80()
{
	__asm
	{
		xor eax, eax
		mov subtitlesCurrent, eax

		push ebx
		push esi
		push edi
		push ebp
		xor ah, ah

		jmp back_00458D86
	}
}

DWORD back_0045ABD9;
DWORD sub_00438B70; // CheckAndPlay
DWORD some_004C0A28;
VOID __declspec(naked) hook_0045ABD4()
{
	__asm {
		mov eax, some_004C0A28
		mov eax, [eax]
		test eax, eax
		jz lbl_back
		call sub_00438B70
		lbl_back : jmp back_0045ABD9
	}
}

DWORD sub_004386F0;
DWORD back_004458F3;
VOID __declspec(naked) hook_004458EE()
{
	__asm
	{
		xor eax, eax
		inc eax
		mov soundIsInventory, eax

		call sub_004386F0
		jmp back_004458F3
	}
}

DWORD back_004409A3;
VOID __declspec(naked) hook_0044099E()
{
	__asm
	{
		xor eax, eax
		inc eax
		mov soundIsInventory, eax

		call sub_004386F0
		jmp back_004409A3
	}
}

namespace Hooks
{
	VOID Patch_Subtitles(HOOKER hooker)
	{
		if (config.language.subtitles < 0)
			return;

		DWORD baseOffset = GetBaseOffset(hooker);

		// Check for subtitles
		PatchHook(hooker, 0x00447831, hook_00447831);
		sub_00448834 = f(0x00448834);
		back_00447836 = f(0x00447836);

		PatchHook(hooker, 0x00445890, hook_00445890);
		sub_00446988 = f(0x00446988);
		back_00445895 = f(0x00445895);

		// Option music - release subtitles
		PatchHook(hooker, 0x00458D80, hook_00458D80);
		back_00458D86 = f(0x00458D86);

		PatchHook(hooker, 0x0045ABD4, hook_0045ABD4);
		back_0045ABD9 = f(0x0045ABD9);
		sub_00438B70 = f(0x00438B70);
		some_004C0A28 = f(0x004C0A28);

		// Set flags for display rectangle
		sub_004386F0 = f(0x004386F0);

		PatchHook(hooker, 0x004458EE, hook_004458EE);
		back_004458F3 = f(0x004458F3);

		PatchHook(hooker, 0x0044099E, hook_0044099E);
		back_004409A3 = f(0x004409A3);

		HANDLE hFile = CreateFile(langFiles.subtitlesFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hFile)
		{
			DWORD dwSize = GetFileSize(hFile, NULL);
			CloseHandle(hFile);

			FILE* hFile = FileOpen(langFiles.subtitlesFile, "rb");
			if (hFile)
			{
				BYTE* fileBuffer = (BYTE*)MemoryAlloc(dwSize);
				if (fileBuffer)
				{
					DWORD count = FileRead(fileBuffer, dwSize, 1, hFile);
					BYTE* ptr = fileBuffer;

					if (*(DWORD*)ptr == 0x00425553) // STR
					{
						ptr += sizeof(DWORD);

						DWORD keysCount = subtitlesCount = *(WORD*)ptr;
						ptr += sizeof(WORD);

						subtitlesList = (SubtitlesItem*)MemoryAlloc(sizeof(SubtitlesItem) * keysCount);

						SubtitlesItem* subItem = subtitlesList;
						while (keysCount--)
						{
							DWORD length = StrLength((CHAR*)ptr);
							StrCopy(subItem->key, (CHAR*)ptr);
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

								DWORD dataLength = 0;
								subLine->text = DecodeUtf8(ptr, &dataLength);
								ptr += dataLength + 1;

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

						if (AddFontResourceC && ptr - fileBuffer != dwSize)
						{
							BYTE* fontPtr = ptr;
							ptr += 16;

							CHAR tempPath[MAX_PATH];
							GetTempPath(MAX_PATH - 1, tempPath);
							DWORD tempLength = StrLength(tempPath);

							// prepare random
							SeedRandom(timeGetTime());

							CHAR familiesList[2][MAX_PATH];
							CHAR* filePath = fontFiles[0];
							DWORD filesCount = *ptr++;
							DWORD fCount = filesCount;
							CHAR* family = familiesList[0];
							do
							{
								DWORD length = StrLength((CHAR*)ptr);
								MemoryCopy(family, ptr, length + 1);
								ptr += length + 1;

								MemoryCopy(filePath, tempPath, tempLength);
								CHAR* ch = filePath + tempLength;
								DWORD count = 10;
								do
								{
									BYTE random = (BYTE)(Random() % ('Z' - 'A' + 11));

									*ch = '0' + random;
									if (*ch > '9')
										*ch = 'A' + (random - 10);

									++ch;
								} while (--count);
								*ch = NULL;
								StrCat(filePath, ".FNT");

								DWORD fileSize = *(DWORD*)ptr;
								ptr += sizeof(DWORD);

								FILE* hFontFIle = FileOpen(filePath, "wb");
								if (hFontFIle)
								{
									FileWrite(ptr, fileSize, 1, hFontFIle);
									FileClose(hFontFIle);

									AddFontResourceC(filePath, FR_PRIVATE, NULL);
								}

								ptr += fileSize;

								family += MAX_PATH;
								filePath += MAX_PATH;
							} while (--fCount);

							SubtitlesFont* fontItem = subtitlesFonts;
							DWORD idx = 0;
							DWORD count = 2;
							do
							{
								DWORD fontSize = *fontPtr++;
								DWORD fontType = *fontPtr++;

								BYTE r = *fontPtr++;
								BYTE g = *fontPtr++;
								BYTE b = *fontPtr++;
								fontItem->color = RGB(r, g, b);

								r = *fontPtr++;
								g = *fontPtr++;
								b = *fontPtr++;
								fontItem->background = RGB(r, g, b);

								fontItem->font = CreateFont(fontSize, 0, 0, 0, (fontType & 0x1) ? 700 : 0, fontType & 0x10, FALSE, FALSE, ANSI_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, familiesList[idx]);

								if (filesCount != 1)
									++idx;

								++fontItem;
							} while (--count);
						}
						else
						{
							SubtitlesFont* fontItem = subtitlesFonts;
							DWORD idx = 0;
							DWORD count = 2;
							do
							{
								fontItem->color = RGB(255, 255, 255);
								fontItem->background = RGB(0, 0, 0);

								fontItem->font = CreateFont(count == 1 ? 24 : 13, 0, 0, 0, 0, 0, FALSE, FALSE, ANSI_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Arial");

								++fontItem;
							} while (--count);
						}
					}

					MemoryFree(fileBuffer);
				}

				FileClose(hFile);
			}
		}
	}
}