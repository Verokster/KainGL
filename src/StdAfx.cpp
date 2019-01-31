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

MALLOC MemoryAlloc;
FREE MemoryFree;
MEMSET MemorySet;
MEMCPY MemoryCopy;
MEMCMP MemoryCompare;
CEIL MathCeil;
FLOOR MathFloor;
ROUND MathRound;
LOG10 MathLog10;
SQRT MathSqrt;
ATAN2 MathAtan2;
SPRINTF StrPrint;
STRSTR StrStr;
STRCHR StrChar;
STRCMP StrCompare;
STRCPY StrCopy;
STRCAT StrCat;
STRDUP StrDuplicate;
STRLEN StrLength;
STRRCHR StrRightChar;
FOPEN FileOpen;
FCLOSE FileClose;
FREAD FileRead;
FWRITE FileWrite;
FSEEK FileSeek;
RAND Random;
SRAND SeedRandom;
EXIT Exit;

XINPUTGETSTATE InputGetState;
XINPUTSETSTATE InputSetState;
XINPUTGETCAPABILITIES InputGetCapabilities;

DWORD
	pAcquireDDThreadLock,
	pCompleteCreateSysmemSurface,
	pD3DParseUnknownCommand,
	pDDGetAttachedSurfaceLcl,
	pDDInternalLock,
	pDDInternalUnlock,
	pDSoundHelp,
	pDirectDrawCreate,
	pDirectDrawCreateClipper,
	pDirectDrawCreateEx,
	pDirectDrawEnumerateA,
	pDirectDrawEnumerateExA,
	pDirectDrawEnumerateExW,
	pDirectDrawEnumerateW,
	pDllCanUnloadNow,
	pDllGetClassObject,
	pGetDDSurfaceLocal,
	pGetOLEThunkData,
	pGetSurfaceFromDC,
	pRegisterSpecialCase,
	pReleaseDDThreadLock,
	pSetAppCompatData;

DIRECTSOUNDCREATE DSCreate;

#define LIBEXP VOID __declspec(naked,nothrow) __stdcall
LIBEXP exAcquireDDThreadLock() { _asm { JMP pAcquireDDThreadLock } }
LIBEXP exCompleteCreateSysmemSurface() { _asm { JMP pCompleteCreateSysmemSurface } }
LIBEXP exD3DParseUnknownCommand() { _asm { JMP pD3DParseUnknownCommand } }
LIBEXP exDDGetAttachedSurfaceLcl() { _asm { JMP pDDGetAttachedSurfaceLcl } }
LIBEXP exDDInternalLock() { _asm { JMP pDDInternalLock } }
LIBEXP exDDInternalUnlock() { _asm { JMP pDDInternalUnlock } }
LIBEXP exDSoundHelp() { _asm { JMP pDSoundHelp } }
LIBEXP exDirectDrawCreate() { _asm { JMP pDirectDrawCreate } }
LIBEXP exDirectDrawCreateClipper() { _asm { JMP pDirectDrawCreateClipper } }
LIBEXP exDirectDrawCreateEx() { _asm { JMP pDirectDrawCreateEx } }
LIBEXP exDirectDrawEnumerateA() { _asm { JMP pDirectDrawEnumerateA } }
LIBEXP exDirectDrawEnumerateExA() { _asm { JMP pDirectDrawEnumerateExA } }
LIBEXP exDirectDrawEnumerateExW() { _asm { JMP pDirectDrawEnumerateExW } }
LIBEXP exDirectDrawEnumerateW() { _asm { JMP pDirectDrawEnumerateW } }
LIBEXP exDllCanUnloadNow() { _asm { JMP pDllCanUnloadNow } }
LIBEXP exDllGetClassObject() { _asm { JMP pDllGetClassObject } }
LIBEXP exGetDDSurfaceLocal() { _asm { JMP pGetDDSurfaceLocal } }
LIBEXP exGetOLEThunkData() { _asm { JMP pGetOLEThunkData } }
LIBEXP exGetSurfaceFromDC() { _asm { JMP pGetSurfaceFromDC } }
LIBEXP exRegisterSpecialCase() { _asm { JMP pRegisterSpecialCase } }
LIBEXP exReleaseDDThreadLock() { _asm { JMP pReleaseDDThreadLock } }
LIBEXP exSetAppCompatData() { _asm { JMP pSetAppCompatData } }

double __cdecl round(double number)
{
	double floorVal = MathFloor(number);
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

VOID LoadMsvCRT()
{
	HMODULE hLib = LoadLibrary("MSVCRT.dll");
	if (hLib)
	{
		StrPrint = (SPRINTF)GetProcAddress(hLib, "sprintf");

		CHAR libName[MAX_PATH];
		for (DWORD i = 12; i >= 7; --i)
		{
			StrPrint(libName, "MSVCR%d0.dll", i);
			HMODULE hCrtLib = LoadLibrary(libName);
			if (hCrtLib)
			{
				FreeLibrary(hLib);
				hLib = hCrtLib;
				StrPrint = (SPRINTF)GetProcAddress(hLib, "sprintf");
				break;
			}
		}

		MemoryAlloc = (MALLOC)GetProcAddress(hLib, "malloc");
		MemoryFree = (FREE)GetProcAddress(hLib, "free");
		MemorySet = (MEMSET)GetProcAddress(hLib, "memset");
		MemoryCopy = (MEMCPY)GetProcAddress(hLib, "memcpy");
		MemoryCompare = (MEMCMP)GetProcAddress(hLib, "memcmp");

		MathCeil = (CEIL)GetProcAddress(hLib, "ceil");
		MathFloor = (FLOOR)GetProcAddress(hLib, "floor");
		MathRound = (ROUND)GetProcAddress(hLib, "round");
		if (!MathRound)
			MathRound = round;
		MathLog10 = (LOG10)GetProcAddress(hLib, "log10");
		MathSqrt = (SQRT)GetProcAddress(hLib, "sqrt");
		MathAtan2 = (ATAN2)GetProcAddress(hLib, "atan2");

		StrStr = (STRSTR)GetProcAddress(hLib, "strstr");
		StrChar = (STRCHR)GetProcAddress(hLib, "strchr");
		StrCompare = (STRCMP)GetProcAddress(hLib, "strcmp");
		StrCopy = (STRCPY)GetProcAddress(hLib, "strcpy");
		StrCat = (STRCAT)GetProcAddress(hLib, "strcat");
		StrDuplicate = (STRDUP)GetProcAddress(hLib, "_strdup");
		StrLength = (STRLEN)GetProcAddress(hLib, "strlen");
		StrRightChar = (STRRCHR)GetProcAddress(hLib, "strrchr");

		FileOpen = (FOPEN)GetProcAddress(hLib, "fopen");
		FileClose = (FCLOSE)GetProcAddress(hLib, "fclose");
		FileRead = (FREAD)GetProcAddress(hLib, "fread");
		FileWrite = (FWRITE)GetProcAddress(hLib, "fwrite");
		FileSeek = (FSEEK)GetProcAddress(hLib, "fseek");

		Random = (RAND)GetProcAddress(hLib, "rand");
		SeedRandom = (SRAND)GetProcAddress(hLib, "srand");

		Exit = (EXIT)GetProcAddress(hLib, "exit");
	}
}

VOID LoadDDraw()
{
	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		StrCat(dir, "\\DDRAW.dll");
		HMODULE hLib = LoadLibrary(dir);
		if (hLib)
		{
			pAcquireDDThreadLock = (DWORD)GetProcAddress(hLib, "AcquireDDThreadLock");
			pCompleteCreateSysmemSurface = (DWORD)GetProcAddress(hLib, "CompleteCreateSysmemSurface");
			pD3DParseUnknownCommand = (DWORD)GetProcAddress(hLib, "D3DParseUnknownCommand");
			pDDGetAttachedSurfaceLcl = (DWORD)GetProcAddress(hLib, "DDGetAttachedSurfaceLcl");
			pDDInternalLock = (DWORD)GetProcAddress(hLib, "DDInternalLock");
			pDDInternalUnlock = (DWORD)GetProcAddress(hLib, "DDInternalUnlock");
			pDSoundHelp = (DWORD)GetProcAddress(hLib, "DSoundHelp");
			pDirectDrawCreate = (DWORD)GetProcAddress(hLib, "DirectDrawCreate");
			pDirectDrawCreateClipper = (DWORD)GetProcAddress(hLib, "DirectDrawCreateClipper");
			pDirectDrawCreateEx = (DWORD)GetProcAddress(hLib, "DirectDrawCreateEx");
			pDirectDrawEnumerateA = (DWORD)GetProcAddress(hLib, "DirectDrawEnumerateA");
			pDirectDrawEnumerateExA = (DWORD)GetProcAddress(hLib, "DirectDrawEnumerateExA");
			pDirectDrawEnumerateExW = (DWORD)GetProcAddress(hLib, "DirectDrawEnumerateExW");
			pDirectDrawEnumerateW = (DWORD)GetProcAddress(hLib, "DirectDrawEnumerateW");
			pDllCanUnloadNow = (DWORD)GetProcAddress(hLib, "DllCanUnloadNow");
			pDllGetClassObject = (DWORD)GetProcAddress(hLib, "DllGetClassObject");
			pGetDDSurfaceLocal = (DWORD)GetProcAddress(hLib, "GetDDSurfaceLocal");
			pGetOLEThunkData = (DWORD)GetProcAddress(hLib, "GetOLEThunkData");
			pGetSurfaceFromDC = (DWORD)GetProcAddress(hLib, "GetSurfaceFromDC");
			pRegisterSpecialCase = (DWORD)GetProcAddress(hLib, "RegisterSpecialCase");
			pReleaseDDThreadLock = (DWORD)GetProcAddress(hLib, "ReleaseDDThreadLock");
			pSetAppCompatData = (DWORD)GetProcAddress(hLib, "SetAppCompatData");
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
