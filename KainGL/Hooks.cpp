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
	VOID __fastcall PatchHook(DWORD addr, VOID* hook)
	{
		BYTE* jump;

		DWORD old_prot;
		VirtualProtect((VOID*)addr, 5, PAGE_EXECUTE_READWRITE, &old_prot);

		jump = (BYTE*)addr;
		*jump = 0xE9;
		++jump;
		*(DWORD*)jump = (DWORD)hook - (DWORD)addr - 5;

		VirtualProtect((VOID*)addr, 5, old_prot, &old_prot);
	}

	VOID __fastcall PatchNop(DWORD addr, DWORD size)
	{
		DWORD old_prot;
		VirtualProtect((VOID*)addr, size, PAGE_EXECUTE_READWRITE, &old_prot);

		memset((VOID*)addr, 0x90, size);

		VirtualProtect((VOID*)addr, size, old_prot, &old_prot);
	}

	VOID __fastcall PatchBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD old_prot;
		VirtualProtect((VOID*)addr, size, PAGE_EXECUTE_READWRITE, &old_prot);

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
			memcpy((VOID*)addr, block, size);
			break;
		}

		VirtualProtect((VOID*)addr, size, old_prot, &old_prot);
	}

	VOID __fastcall ReadBlock(DWORD addr, VOID* block, DWORD size)
	{
		DWORD old_prot;
		VirtualProtect((VOID*)addr, size, PAGE_READONLY, &old_prot);

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
			memcpy(block, (VOID*)addr, size);
			break;
		}

		VirtualProtect((VOID*)addr, size, old_prot, &old_prot);
	}

	VOID __fastcall PatchWord(DWORD addr, WORD value)
	{
		PatchBlock(addr, &value, sizeof(value));
	}

	VOID __fastcall PatchInt(DWORD addr, INT value)
	{
		PatchBlock(addr, &value, sizeof(value));
	}

	VOID __fastcall PatchDWord(DWORD addr, DWORD value)
	{
		PatchBlock(addr, &value, sizeof(value));
	}

	VOID __fastcall PatchByte(DWORD addr, BYTE value)
	{
		PatchBlock(addr, &value, sizeof(value));
	}

	DWORD __fastcall ReadDWord(DWORD addr)
	{
		DWORD value;
		ReadBlock(addr, &value, sizeof(value));
		return value;
	}

	BOOL Load()
	{
		if (ReadDWord(0x0045F5DA + 1) == WS_POPUP)
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