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

namespace Hooks
{
	BOOL __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		DWORD old_prot;
		if (VirtualProtect((VOID*)addr, 5, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)addr;
			*jump = 0xE9;
			++jump;
			*(DWORD*)jump = (DWORD)hook - (DWORD)addr - 5;

			VirtualProtect((VOID*)addr, 5, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD old_prot;
		if (VirtualProtect((VOID*)addr, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			MemorySet((VOID*)addr, 0x90, size);
			VirtualProtect((VOID*)addr, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD old_prot;
		if (VirtualProtect((VOID*)addr, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)addr = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)addr = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)addr = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)addr, block, size);
				break;
			}

			VirtualProtect((VOID*)addr, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD old_prot;
		if (VirtualProtect((VOID*)addr, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)addr;
				break;
			case 2:
				*(WORD*)block = *(WORD*)addr;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)addr;
				break;
			default:
				MemoryCopy(block, (VOID*)addr, size);
				break;
			}

			VirtualProtect((VOID*)addr, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchWord(DWORD addr, WORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchInt(DWORD addr, INT value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall PatchByte(DWORD addr, BYTE value)
	{
		return PatchBlock(addr, &value, sizeof(value));
	}

	BOOL __fastcall ReadWord(DWORD addr, WORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall ReadDWord(DWORD addr, DWORD* value)
	{
		return ReadBlock(addr, value, sizeof(*value));
	}

	BOOL __fastcall PatchFunction(const CHAR* function, VOID* addr)
	{
		BOOL res = FALSE;

		PIMAGE_DOS_HEADER headDOS = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)headDOS + headDOS->e_lfanew);

		PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)headDOS + headNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		for (; imports->Name; ++imports)
		{
			CHAR* libraryName = (CHAR*)((DWORD)headDOS + imports->Name);
			PIMAGE_THUNK_DATA nameThunk = (PIMAGE_THUNK_DATA)((DWORD)headDOS + imports->OriginalFirstThunk);
			PIMAGE_THUNK_DATA addressThunk = (PIMAGE_THUNK_DATA)((DWORD)headDOS + imports->FirstThunk);
			for (; nameThunk->u1.AddressOfData; ++nameThunk, ++addressThunk)
			{
				DWORD name = (DWORD)headDOS + nameThunk->u1.AddressOfData;
				CHAR* funcName = (CHAR*)(name + 2);

				WORD indent = 0;
				if (ReadWord(name, (WORD*)&indent) && !strcmp(funcName, function))
				{
					DWORD res;
					if (ReadDWord((DWORD)&addressThunk->u1.AddressOfData, &res) && PatchDWord((DWORD)&addressThunk->u1.AddressOfData, (DWORD)addr))
						res = TRUE;
				}
			}
		}

		return res;
	}

	BOOL Load()
	{
		DWORD check;
		if (ReadDWord(0x0045F5DA + 1, &check) && check == WS_POPUP)
		{
			Patch_Library();
			Patch_System();
			Patch_Window();
			Patch_Audio();
			Patch_Mouse();
			Patch_NoCD();
			Patch_Language();
			Patch_Zoom();
			return TRUE;
		}

		return FALSE;
	}
}