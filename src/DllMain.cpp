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
#include "Resource.h"
#include "Main.h"
#include "GLib.h"
#include "ALib.h"
#include "OpenDraw.h"
#include "OpenSound.h"

HHOOK OldWindowKeyHook;
LRESULT __stdcall WindowKeyHook(INT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		KBDLLHOOKSTRUCT* phs = (KBDLLHOOKSTRUCT*)lParam;
		if (phs->vkCode == VK_SNAPSHOT)
		{
			HWND hWnd = GetActiveWindow();
			OpenDraw* draw = Main::FindDrawByWindow(hWnd);
			if (draw && !configDisplayWindowed)
			{
				draw->isTakeSnapshot = TRUE;
				if (draw)
					SetEvent(draw->hDrawEvent);

				return TRUE;
			}
		}
	}

	return CallNextHookEx(OldWindowKeyHook, nCode, wParam, lParam);
}

BOOL __stdcall DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		LoadMsvCRT();
		LoadDDraw();
		LoadDSound();
		LoadXInput();

		hDllModule = hModule;
		if (Hooks::Load())
		{
			LoadKernel32();

			AL::Load();

			OldWindowKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, WindowKeyHook, NULL, 0);

			if (CreateActCtxC)
			{
				CHAR path[MAX_PATH];
				GetModuleFileName(hModule, path, MAX_PATH - 1);

				ACTCTX actCtx;
				MemoryZero(&actCtx, sizeof(ACTCTX));
				actCtx.cbSize = sizeof(ACTCTX);
				actCtx.lpSource = path;
				actCtx.hModule = hModule;
				actCtx.lpResourceName = MAKEINTRESOURCE(IDM_MANIFEST);
				actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
				hActCtx = CreateActCtxC(&actCtx);
			}
		}
		else
			hDllModule = NULL;
		
		break;

	case DLL_PROCESS_DETACH:
		if (hDllModule)
		{
			if (hActCtx && hActCtx != INVALID_HANDLE_VALUE)
				ReleaseActCtxC(hActCtx);

			UnhookWindowsHookEx(OldWindowKeyHook);
			ChangeDisplaySettings(NULL, NULL);
		}

		break;

	default: break;
	}
	return TRUE;
}