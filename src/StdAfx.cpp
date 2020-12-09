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

#include "StdAfx.h"

HMODULE hDllModule;
HANDLE hActCtx;

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

DRAWTEXTW DrawTextUni;

DWMISCOMPOSITIONENABLED IsDwmCompositionEnabled;

ADDFONTRESOURCEEXA AddFontResourceC;
REMOVEFONTRESOURCEEXA RemoveFontResourceC;

XINPUTGETSTATE InputGetState;
XINPUTSETSTATE InputSetState;
XINPUTGETCAPABILITIES InputGetCapabilities;

DIRECTSOUNDCREATE DSCreate;

#define LIBEXP(a) DWORD p##a; VOID __declspec(naked,nothrow) __stdcall ex##a() { LoadDDraw(); _asm { jmp p##a } }
#define LIBLOAD(lib, a) p##a = (DWORD)GetProcAddress(lib, #a);

LIBEXP(AcquireDDThreadLock)
LIBEXP(CompleteCreateSysmemSurface)
LIBEXP(D3DParseUnknownCommand)
LIBEXP(DDGetAttachedSurfaceLcl)
LIBEXP(DDInternalLock)
LIBEXP(DDInternalUnlock)
LIBEXP(DSoundHelp)
LIBEXP(DirectDrawCreate)
LIBEXP(DirectDrawCreateClipper)
LIBEXP(DirectDrawCreateEx)
LIBEXP(DirectDrawEnumerateA)
LIBEXP(DirectDrawEnumerateExA)
LIBEXP(DirectDrawEnumerateExW)
LIBEXP(DirectDrawEnumerateW)
LIBEXP(DllCanUnloadNow)
LIBEXP(DllGetClassObject)
LIBEXP(GetDDSurfaceLocal)
LIBEXP(GetOLEThunkData)
LIBEXP(GetSurfaceFromDC)
LIBEXP(RegisterSpecialCase)
LIBEXP(ReleaseDDThreadLock)
LIBEXP(SetAppCompatData)

DOUBLE __fastcall MathRound(DOUBLE number)
{
	DOUBLE floorVal = MathFloor(number);
	return floorVal + 0.5f > number ? floorVal : MathCeil(number);
}

VOID LoadKernel32()
{
	HMODULE hLib = GetModuleHandle("KERNEL32.dll");
	if (hLib)
	{
		CreateActCtxC = (CREATEACTCTXA)GetProcAddress(hLib, "CreateActCtxA");
		ReleaseActCtxC = (RELEASEACTCTX)GetProcAddress(hLib, "ReleaseActCtx");
		ActivateActCtxC = (ACTIVATEACTCTX)GetProcAddress(hLib, "ActivateActCtx");
		DeactivateActCtxC = (DEACTIVATEACTCTX)GetProcAddress(hLib, "DeactivateActCtx");
	}
}

VOID LoadGdi32()
{
	HMODULE hLib = GetModuleHandle("GDI32.dll");
	if (hLib)
	{
		AddFontResourceC = (ADDFONTRESOURCEEXA)GetProcAddress(hLib, "AddFontResourceExA");
		RemoveFontResourceC = (REMOVEFONTRESOURCEEXA)GetProcAddress(hLib, "RemoveFontResourceExA");
	}
}

VOID LoadUnicoWS()
{
	if ((GetVersion() & 0xFF) < 5)
	{
		HMODULE hLib = LoadLibrary("UNICOWS.dll");
		if (hLib)
			DrawTextUni = (DRAWTEXTW)GetProcAddress(hLib, "DrawTextW");
	}

	if (!DrawTextUni)
		DrawTextUni = DrawTextW;
}

VOID LoadDwmAPI()
{
	HMODULE hLib = LoadLibrary("DWMAPI.dll");
	if (hLib)
		IsDwmCompositionEnabled = (DWMISCOMPOSITIONENABLED)GetProcAddress(hLib, "DwmIsCompositionEnabled");
}

VOID LoadDDraw()
{
	static BOOL isLoaded;

	if (!isLoaded)
	{
		isLoaded = TRUE;

		CHAR dir[MAX_PATH];
		if (GetSystemDirectory(dir, MAX_PATH))
		{
			StrCat(dir, "\\DDRAW.dll");
			HMODULE hLib = LoadLibrary(dir);
			if (hLib)
			{
				LIBLOAD(hLib, AcquireDDThreadLock);
				LIBLOAD(hLib, CompleteCreateSysmemSurface);
				LIBLOAD(hLib, D3DParseUnknownCommand);
				LIBLOAD(hLib, DDGetAttachedSurfaceLcl);
				LIBLOAD(hLib, DDInternalLock);
				LIBLOAD(hLib, DDInternalUnlock);
				LIBLOAD(hLib, DSoundHelp);
				LIBLOAD(hLib, DirectDrawCreate);
				LIBLOAD(hLib, DirectDrawCreateClipper);
				LIBLOAD(hLib, DirectDrawCreateEx);
				LIBLOAD(hLib, DirectDrawEnumerateA);
				LIBLOAD(hLib, DirectDrawEnumerateExA);
				LIBLOAD(hLib, DirectDrawEnumerateExW);
				LIBLOAD(hLib, DirectDrawEnumerateW);
				LIBLOAD(hLib, DllCanUnloadNow);
				LIBLOAD(hLib, DllGetClassObject);
				LIBLOAD(hLib, GetDDSurfaceLocal);
				LIBLOAD(hLib, GetOLEThunkData);
				LIBLOAD(hLib, GetSurfaceFromDC);
				LIBLOAD(hLib, RegisterSpecialCase);
				LIBLOAD(hLib, ReleaseDDThreadLock);
				LIBLOAD(hLib, SetAppCompatData);
			}
		}
	}
}

VOID LoadXInput()
{
	CHAR libName[MAX_PATH];

	INT i = 4;
	do
	{
		StrPrint(libName, "XInput%s1_%d.dll", i ? "" : "9_", i);
		HMODULE hLib = LoadLibrary(libName);
		if (hLib)
		{
			InputGetState = (XINPUTGETSTATE)GetProcAddress(hLib, "XInputGetState");
			InputSetState = (XINPUTSETSTATE)GetProcAddress(hLib, "XInputSetState");
			InputGetCapabilities = (XINPUTGETCAPABILITIES)GetProcAddress(hLib, "XInputGetCapabilities");
			return;
		}
	} while (i--);
}

VOID LoadShcore()
{
	HMODULE hLib = LoadLibrary("SHCORE.dll");
	if (hLib)
	{

		SETPROCESSDPIAWARENESS SetProcessDpiAwarenessC = (SETPROCESSDPIAWARENESS)GetProcAddress(hLib, "SetProcessDpiAwareness");
		if (SetProcessDpiAwarenessC)
			SetProcessDpiAwarenessC(PROCESS_PER_MONITOR_DPI_AWARE);
	}
}

WCHAR* __fastcall DecodeUtf8(BYTE* ptr, DWORD* count)
{
	DWORD length;
	return DecodeUtf8(ptr, count, &length);
}

WCHAR* __fastcall DecodeUtf8(BYTE* ptr, DWORD* count, DWORD* length)
{
	*length = 0;
	DWORD idx = 0;
	BYTE* ch = ptr;

	if (*count)
	{
		while (idx != *count)
		{
			if (!(*ch & 0x80) || (*ch & 0xC0) == 0xC0)
				++*length;

			++idx;
			++ch;
		}
	}
	else
	{
		while (*ch)
		{
			if (!(*ch & 0x80) || (*ch & 0xC0) == 0xC0)
				++*length;

			++idx;
			++ch;
		}

		*count = idx;
	}

	WCHAR* text = (WCHAR*)MemoryAlloc((*length + 1) << 1);
	*(text + *length) = 0;

	DWORD code = 0;
	idx = 0;
	ch = ptr;
	WORD* str = (WORD*)text;
	while (idx != *count)
	{
		DWORD check = 0x80;
		if (*ch & check)
		{
			check >>= 1;
			if (*ch & check)
			{
				if (idx)
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
			if (idx)
				*str++ = LOWORD(code);

			code = *ch;
		}

		++idx;
		++ch;
	}

	if (idx)
		*str++ = LOWORD(code);

	return text;
}
