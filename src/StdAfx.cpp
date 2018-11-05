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

#include "StdAfx.h"

HMODULE hDllModule;
HANDLE hActCtx;

CREATEACTCTXA CreateActCtxC;
RELEASEACTCTX ReleaseActCtxC;
ACTIVATEACTCTX ActivateActCtxC;
DEACTIVATEACTCTX DeactivateActCtxC;

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
STRCMP StrCompare;
STRCPY StrCopy;
STRCAT StrCat;
STRLEN StrLength;
STRRCHR StrRightChar;
FOPEN FileOpen;
FCLOSE FileClose;
FREAD FileRead;
RAND Random;
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
//pDllCanUnloadNow,
//pDllGetClassObject,
pGetDDSurfaceLocal,
pGetOLEThunkData,
pGetSurfaceFromDC,
pRegisterSpecialCase,
pReleaseDDThreadLock,
pSetAppCompatData;

DIRECTSOUNDCREATE DSCreate;

#define LIBEXP __declspec(naked,nothrow) __stdcall
VOID LIBEXP AcquireDDThreadLock() { _asm { JMP pAcquireDDThreadLock } }
VOID LIBEXP CompleteCreateSysmemSurface() { _asm { JMP pCompleteCreateSysmemSurface } }
VOID LIBEXP D3DParseUnknownCommand() { _asm { JMP pD3DParseUnknownCommand } }
VOID LIBEXP DDGetAttachedSurfaceLcl() { _asm { JMP pDDGetAttachedSurfaceLcl } }
VOID LIBEXP DDInternalLock() { _asm { JMP pDDInternalLock } }
VOID LIBEXP DDInternalUnlock() { _asm { JMP pDDInternalUnlock } }
VOID LIBEXP DSoundHelp() { _asm { JMP pDSoundHelp } }
VOID LIBEXP DirectDrawCreate() { _asm { JMP pDirectDrawCreate } }
VOID LIBEXP DirectDrawCreateClipper() { _asm { JMP pDirectDrawCreateClipper } }
VOID LIBEXP DirectDrawCreateEx() { _asm { JMP pDirectDrawCreateEx } }
VOID LIBEXP DirectDrawEnumerateA() { _asm { JMP pDirectDrawEnumerateA } }
VOID LIBEXP DirectDrawEnumerateExA() { _asm { JMP pDirectDrawEnumerateExA } }
VOID LIBEXP DirectDrawEnumerateExW() { _asm { JMP pDirectDrawEnumerateExW } }
VOID LIBEXP DirectDrawEnumerateW() { _asm { JMP pDirectDrawEnumerateW } }
//VOID LIBEXP DllCanUnloadNow() { _asm { JMP pDllCanUnloadNow } }
//VOID LIBEXP DllGetClassObject() { _asm { JMP pDllGetClassObject } }
VOID LIBEXP GetDDSurfaceLocal() { _asm { JMP pGetDDSurfaceLocal } }
VOID LIBEXP GetOLEThunkData() { _asm { JMP pGetOLEThunkData } }
VOID LIBEXP GetSurfaceFromDC() { _asm { JMP pGetSurfaceFromDC } }
VOID LIBEXP RegisterSpecialCase() { _asm { JMP pRegisterSpecialCase } }
VOID LIBEXP ReleaseDDThreadLock() { _asm { JMP pReleaseDDThreadLock } }
VOID LIBEXP SetAppCompatData() { _asm { JMP pSetAppCompatData } }

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
		StrCompare = (STRCMP)GetProcAddress(hLib, "strcmp");
		StrCopy = (STRCPY)GetProcAddress(hLib, "strcpy");
		StrCat = (STRCAT)GetProcAddress(hLib, "strcat");
		StrLength = (STRLEN)GetProcAddress(hLib, "strlen");
		StrRightChar = (STRRCHR)GetProcAddress(hLib, "strrchr");

		FileOpen = (FOPEN)GetProcAddress(hLib, "fopen");
		FileClose = (FCLOSE)GetProcAddress(hLib, "fclose");
		FileRead = (FREAD)GetProcAddress(hLib, "fread");

		Random = (RAND)GetProcAddress(hLib, "rand");

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
			//pDllCanUnloadNow = (DWORD)GetProcAddress(hLib, "DllCanUnloadNow");
			//pDllGetClassObject = (DWORD)GetProcAddress(hLib, "DllGetClassObject");
			pGetDDSurfaceLocal = (DWORD)GetProcAddress(hLib, "GetDDSurfaceLocal");
			pGetOLEThunkData = (DWORD)GetProcAddress(hLib, "GetOLEThunkData");
			pGetSurfaceFromDC = (DWORD)GetProcAddress(hLib, "GetSurfaceFromDC");
			pRegisterSpecialCase = (DWORD)GetProcAddress(hLib, "RegisterSpecialCase");
			pReleaseDDThreadLock = (DWORD)GetProcAddress(hLib, "ReleaseDDThreadLock");
			pSetAppCompatData = (DWORD)GetProcAddress(hLib, "SetAppCompatData");
		}
	}
}

VOID LoadDSound()
{
	HMODULE hLib = LoadLibrary("DSOUND.dll");
	if (hLib)
		DSCreate = (DIRECTSOUNDCREATE)GetProcAddress(hLib, "DirectSoundCreate");
}

VOID LoadXInput()
{
	CHAR libName[MAX_PATH];
	for (INT i = 4; i >= 0; --i)
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
	}
}