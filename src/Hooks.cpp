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
#include "CommCtrl.h"

CHAR kainDirPath[MAX_PATH];

namespace Hooks
{
	PIMAGE_DOS_HEADER headDOS;
	INT baseOffset;
	HWND hMainWnd;

	DWORD sub_GetHash = 0x0041279C;

	BOOL __fastcall PatchRedirect(DWORD addr, DWORD dest, DWORD size, BYTE instruction)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			BYTE* jump = (BYTE*)address;
			*jump = instruction;
			++jump;
			*(DWORD*)jump = dest - address - size;

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchJump(DWORD addr, DWORD dest)
	{
		INT relative = dest - addr - baseOffset - 2;
		if (relative >= -128 && relative <= 127)
			return PatchRedirect(addr, dest, 2, 0xEB);
		else
			return PatchRedirect(addr, dest, 5, 0xE9);
	}

	BOOL __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, (DWORD)hook, 5, 0xE9);
	}

	BOOL __fastcall PatchCall(DWORD addr, VOID* hook)
	{
		return PatchRedirect(addr, (DWORD)hook, 5, 0xE8);
	}

	BOOL __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			MemorySet((VOID*)address, 0x90, size);
			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_EXECUTE_READWRITE, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)address = *(DWORD*)block;
				break;
			case 2:
				*(WORD*)address = *(WORD*)block;
				break;
			case 1:
				*(BYTE*)address = *(BYTE*)block;
				break;
			default:
				MemoryCopy((VOID*)address, block, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

			return TRUE;
		}
		return FALSE;
	}

	BOOL __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD address = addr + baseOffset;

		DWORD old_prot;
		if (VirtualProtect((VOID*)address, size, PAGE_READONLY, &old_prot))
		{
			switch (size)
			{
			case 4:
				*(DWORD*)block = *(DWORD*)address;
				break;
			case 2:
				*(WORD*)block = *(WORD*)address;
				break;
			case 1:
				*(BYTE*)block = *(BYTE*)address;
				break;
			default:
				MemoryCopy(block, (VOID*)address, size);
				break;
			}

			VirtualProtect((VOID*)address, size, old_prot, &old_prot);

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

	DWORD __fastcall PatchFunction(const CHAR* function, VOID* addr)
	{
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

				DWORD res = NULL;
				WORD indent = 0;
				if (ReadWord((INT)name - baseOffset, (WORD*)&indent) && !StrCompare(funcName, function) &&
					ReadDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, &res) &&
					PatchDWord((INT)&addressThunk->u1.AddressOfData - baseOffset, (DWORD)addr))
						return res;
			}
		}

		return NULL;
	}

	VOID Start()
	{
		LoadXInput();
		LoadGdi32();
		LoadUnicoWS();
		LoadDwmAPI();
		// -------------
		Patch_Window();
		// -------------
		Patch_Library();
		Patch_Video();
		Patch_Trailer();
		Patch_Subtitles();
		Patch_System();
		Patch_Timers();
		Patch_Audio();
		Patch_Mouse();
		Patch_NoCD();
		Patch_Language();
		Patch_Zoom();
		Patch_EagleEye();
		Patch_Modes();
		Patch_Input();
	}

	DWORD back_0046B6AC = 0x0046B6AC;
	VOID __declspec(naked) hook_0046841E()
	{
		_asm
		{
			CALL Start
			JMP back_0046B6AC
		}
	}

	BOOL Load()
	{
		headDOS = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
		PIMAGE_NT_HEADERS headNT = (PIMAGE_NT_HEADERS)((BYTE*)headDOS + headDOS->e_lfanew);
		baseOffset = (INT)headDOS - (INT)headNT->OptionalHeader.ImageBase;

		DWORD check;
		if (ReadDWord(0x0045F5DA + 1, &check) && check == WS_POPUP)
		{
			GetModuleFileName(NULL, kainDirPath, sizeof(kainDirPath) - 1);
			CHAR* p = StrLastChar(kainDirPath, '\\');
			if (!p)
				p = kainDirPath;
			StrCopy(p, "\\KAIN");

			configCameraStatic = (BOOL*)(0x005947CC + baseOffset);

			PatchHook(0x0046841E, hook_0046841E);
			back_0046B6AC += baseOffset;

			sub_GetHash += baseOffset;

			return TRUE;
		}

		return FALSE;
	}
}