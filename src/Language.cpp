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

#define MAX_FILES_COUNT 0x8000

LangFiles langFiles;

DWORD* filesCount = (DWORD*)0x008E2E20;
FileHeader* filesHeaders = (FileHeader*)0x00506AA4;
FILE** filesHandlers = (FILE**)0x00501324;

CHAR* pillBigPath = (CHAR*)0x005012A4;
CHAR audioBigPath[MAX_PATH];

CHAR* bigPathes[MAX_FILES_COUNT];

VOID __fastcall LoadLangBigFile(CHAR* filePath)
{
	if (!*filePath)
		return;

	FILE* hFile = FileOpen(filePath, "rb");
	if (hFile)
	{
		DWORD count;
		if (FileRead(&count, sizeof(DWORD), 1, hFile) && count > 0 && count < MAX_FILES_COUNT)
		{
			DWORD size = count * sizeof(FileHeader);
			FileHeader* headers = (FileHeader*)MemoryAlloc(size);
			if (headers)
			{
				if (FileRead(headers, size, 1, hFile))
				{
					FileHeader* src = headers;
					do
					{
						BOOL found = FALSE;

						FileHeader* dst = filesHeaders;

						DWORD fCount = *filesCount;
						while (fCount--)
						{
							if (dst->hash == src->hash)
							{
								dst->size = src->size;
								dst->offset = src->offset;
								bigPathes[*filesCount - fCount - 1] = filePath;
								found = TRUE;
								break;
							}

							++dst;
						}

						if (!found)
						{
							filesHeaders[*filesCount] = *src;
							bigPathes[*filesCount] = filePath;

							++(*filesCount);
						}

						++src;
					} while (--count);
				}

				MemoryFree(headers);
			}
		}

		FileClose(hFile);
	}
}

VOID LoadBigFiles()
{
	for (DWORD i = 0; i < MAX_FILES_COUNT; ++i)
		bigPathes[i] = pillBigPath;

	StrPrint(audioBigPath, "%s\\%s", kainDirPath, "AUDIO.BIG");
	LoadLangBigFile(audioBigPath);

	LoadLangBigFile(langFiles.voicesFile);
	LoadLangBigFile(langFiles.interfaceFile);

	Hooks::Patch_Credits();
}

VOID __declspec(naked) hook_00412856()
{
	__asm
	{
		CALL LoadBigFiles
		POP EBX
		XOR EAX, EAX
		INC EAX
		RETN
	}
}

VOID __stdcall GetFilePath(DWORD index, CHAR** outPath)
{
	*outPath = bigPathes[index];
}

DWORD back_00412C8E = 0x00412C8E;
VOID __declspec(naked) hook_00412C89()
{
	__asm
	{
		PUSH EAX
		PUSH ESP
		PUSH EBX
		CALL GetFilePath
		JMP back_00412C8E
	}
}

DWORD back_00412D70 = 0x00412D70;
VOID __declspec(naked) hook_00412D6B()
{
	__asm
	{
		PUSH EAX
		PUSH ESP
		PUSH EBX
		CALL GetFilePath
		JMP back_00412D70
	}
}

DWORD back_00412DDE = 0x00412DDE;
VOID __declspec(naked) hook_00412DD9()
{
	__asm
	{
		PUSH EAX
		PUSH ESP
		PUSH EBX
		CALL GetFilePath
		JMP back_00412DDE
	}
}

DWORD back_00412F8C = 0x00412F8C;
VOID __declspec(naked) hook_00412F87()
{
	__asm
	{
		PUSH EAX
		PUSH ESP
		PUSH EBX
		CALL GetFilePath
		JMP back_00412F8C
	}
}

DWORD back_00413163 = 0x00413163;
VOID __declspec(naked) hook_0041315E()
{
	__asm
	{
		PUSH EAX
		PUSH ESP
		PUSH ESI
		CALL GetFilePath
		JMP back_00413163
	}
}

namespace Hooks
{
	VOID Patch_Language()
	{
		filesCount = (DWORD*)((DWORD)filesCount + baseOffset);
		filesHeaders = (FileHeader*)((DWORD)filesHeaders + baseOffset);
		filesHandlers = (FILE**)((DWORD)filesHandlers + baseOffset);

		pillBigPath = (CHAR*)((DWORD)pillBigPath + baseOffset);

		PatchByte(0x0044E669, 0xEB); // remove language check

		PatchHook(0x00412856, hook_00412856); // Load other big files

		// retrive big pathes by files index
		PatchHook(0x00412C89, hook_00412C89);
		back_00412C8E += baseOffset;

		PatchHook(0x00412D6B, hook_00412D6B);
		back_00412D70 += baseOffset;

		PatchHook(0x00412DD9, hook_00412DD9);
		back_00412DDE += baseOffset;

		PatchHook(0x00412F87, hook_00412F87);
		back_00412F8C += baseOffset;

		PatchHook(0x0041315E, hook_0041315E);
		back_00413163 += baseOffset;
	}
}