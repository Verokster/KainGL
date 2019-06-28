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
#include "OpenDraw.h"

namespace Hooks
{
	BOOL __stdcall PeekMessageHook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		OpenDraw* ddraw = ddrawList;
		if (ddraw)
			ddraw->CheckSyncDraw();

		return PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}

	LSTATUS __stdcall RegQueryValueExHook(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
	{
		if (!StrCompare(lpValueName, "JAMPath"))
		{
			if (RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData) != ERROR_SUCCESS)
				StrCopy((CHAR*)lpData, kainDirPath);
			return ERROR_SUCCESS;
		}
		else if (!StrCompare(lpValueName, "BIGPath"))
		{
			CHAR bigPath[MAX_PATH];
			StrCopy(bigPath, kainDirPath);
			StrCat(bigPath, "\\PILL.BIG");

			FILE* hFile = FileOpen(bigPath, "rb");
			if (hFile)
			{
				FileClose(hFile);
				StrCopy((CHAR*)lpData, kainDirPath);
				return ERROR_SUCCESS;
			}
		}
		else if (!StrCompare(lpValueName, "SAVPath"))
		{
			if (RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData) != ERROR_SUCCESS)
			{
				CHAR* path = (CHAR*)lpData;
				GetModuleFileName(NULL, path, *lpcbData - 1);
				CHAR* p = StrLastChar(path, '\\');
				if (!p)
					p = path;
				StrCopy(p, "\\SAVED");
			}

			return ERROR_SUCCESS;
		}

		return RegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	}

	VOID Patch_System()
	{
		PatchByte(0x00429430, 0x75);
		PatchNop(0x0044CAC4, 2);
		PatchByte(0x00468C6C, 0xC3);
		
		PatchByte(0x00467C74, 0x7E); // patch resolution count check

		if (configSingleThread)
			PatchFunction("PeekMessageA", PeekMessageHook);

		PatchFunction("RegQueryValueExA", RegQueryValueExHook);
	}
}