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
#include "CommCtrl.h"

CHAR kainDirPath[MAX_PATH];
CHAR kainJamPath[MAX_PATH];

namespace Hooks
{
	HWND hMainWnd;

	DWORD sub_GetHash;

	VOID Start()
	{
		LoadXInput();
		LoadGdi32();
		LoadUnicoWS();
		LoadDwmAPI();
		LoadShcore();

		HOOKER hooker = CreateHooker(GetModuleHandle(NULL));
		{
			// -------------
			Patch_Window(hooker);
			// -------------
			Patch_Library(hooker);
			Patch_Video(hooker);
			Patch_Trailer(hooker);
			Patch_Subtitles(hooker);
			Patch_System(hooker);
			Patch_Timers(hooker);
			Patch_Audio(hooker);
			Patch_Mouse(hooker);
			Patch_NoCD(hooker);
			Patch_Language(hooker);
			Patch_Zoom(hooker);
			Patch_EagleEye(hooker);
			Patch_Modes(hooker);
			Patch_Input(hooker);
			Patch_Image(hooker);
		}
		ReleaseHooker(hooker);
	}

	DWORD back_0046B6AC;
	VOID __declspec(naked) hook_0046841E()
	{
		_asm
		{
			call Start
			jmp back_0046B6AC
		}
	}

	BOOL Load()
	{
		HOOKER hooker = CreateHooker(GetModuleHandle(NULL));
		{
			DWORD baseOffset = GetBaseOffset(hooker);

			DWORD check;
			if (ReadDWord(hooker, 0x0045F5DA + 1, &check) && check == WS_POPUP)
			{
				GetModuleFileName(NULL, kainDirPath, sizeof(kainDirPath) - 1);
				CHAR* p = StrLastChar(kainDirPath, '\\');
				if (!p)
					p = kainDirPath;
				StrCopy(p, "\\KAIN");

				BOOL found = FALSE;
				HKEY phkResult;
				if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, KEY_ALL_ACCESS, &phkResult) == ERROR_SUCCESS)
				{
					HKEY hKey;
					if (RegOpenKeyExA(phkResult, "LegacyofKain", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
					{
						DWORD size = sizeof(kainJamPath);
						found = RegQueryValueExA(hKey, "JAMPath", 0, 0, (BYTE*)kainJamPath, &size) == ERROR_SUCCESS;

						RegCloseKey(hKey);
					}

					RegCloseKey(phkResult);
				}

				if (!found)
					StrCopy(kainJamPath, kainDirPath);

				config.camera.isStatic = (BOOL*)(0x005947CC + baseOffset);

				DWORD old_prot;
				VirtualProtect((VOID*)(0x00480000 + baseOffset), 0x2000, PAGE_EXECUTE_READWRITE, &old_prot);

				PatchHook(hooker, 0x0046841E, hook_0046841E);
				back_0046B6AC = f(0x0046B6AC);

				sub_GetHash = f(0x0041279C);

				return TRUE;
			}
		}
		ReleaseHooker(hooker);

		return FALSE;
	}
}