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

DWORD libCount;
HMODULE* modules;

HMODULE hDllModule;

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

VOID _declspec(naked) __stdcall AcquireDDThreadLock() { _asm { JMP pAcquireDDThreadLock } }
VOID _declspec(naked) __stdcall CompleteCreateSysmemSurface() { _asm { JMP pCompleteCreateSysmemSurface } }
VOID _declspec(naked) __stdcall D3DParseUnknownCommand() { _asm { JMP pD3DParseUnknownCommand } }
VOID _declspec(naked) __stdcall DDGetAttachedSurfaceLcl() { _asm { JMP pDDGetAttachedSurfaceLcl } }
VOID _declspec(naked) __stdcall DDInternalLock() { _asm { JMP pDDInternalLock } }
VOID _declspec(naked) __stdcall DDInternalUnlock() { _asm { JMP pDDInternalUnlock } }
VOID _declspec(naked) __stdcall DSoundHelp() { _asm { JMP pDSoundHelp } }
VOID _declspec(naked) __stdcall DirectDrawCreate() { _asm { JMP pDirectDrawCreate } }
VOID _declspec(naked) __stdcall DirectDrawCreateClipper() { _asm { JMP pDirectDrawCreateClipper } }
VOID _declspec(naked) __stdcall DirectDrawCreateEx() { _asm { JMP pDirectDrawCreateEx } }
VOID _declspec(naked) __stdcall DirectDrawEnumerateA() { _asm { JMP pDirectDrawEnumerateA } }
VOID _declspec(naked) __stdcall DirectDrawEnumerateExA() { _asm { JMP pDirectDrawEnumerateExA } }
VOID _declspec(naked) __stdcall DirectDrawEnumerateExW() { _asm { JMP pDirectDrawEnumerateExW } }
VOID _declspec(naked) __stdcall DirectDrawEnumerateW() { _asm { JMP pDirectDrawEnumerateW } }
//VOID _declspec(naked) __stdcall DllCanUnloadNow() { _asm { JMP pDllCanUnloadNow } }
//VOID _declspec(naked) __stdcall DllGetClassObject() { _asm { JMP pDllGetClassObject } }
VOID _declspec(naked) __stdcall GetDDSurfaceLocal() { _asm { JMP pGetDDSurfaceLocal } }
VOID _declspec(naked) __stdcall GetOLEThunkData() { _asm { JMP pGetOLEThunkData } }
VOID _declspec(naked) __stdcall GetSurfaceFromDC() { _asm { JMP pGetSurfaceFromDC } }
VOID _declspec(naked) __stdcall RegisterSpecialCase() { _asm { JMP pRegisterSpecialCase } }
VOID _declspec(naked) __stdcall ReleaseDDThreadLock() { _asm { JMP pReleaseDDThreadLock } }
VOID _declspec(naked) __stdcall SetAppCompatData() { _asm { JMP pSetAppCompatData } }

VOID LoadRealLibrary()
{
	CHAR dir[MAX_PATH];
	if (GetSystemDirectory(dir, MAX_PATH))
	{
		strcat(dir, "\\DDRAW.dll");
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		hDllModule = hModule;
		DisableThreadLibraryCalls(hModule);

		if (!Hooks::Load())
			return FALSE;

		LoadRealLibrary();
		Config::Load();
		break;
	}

	case DLL_PROCESS_DETACH:
	{
		ChangeDisplaySettings(NULL, NULL);
		GL::Free();
		break;
	}

	default: break;
	}
	return TRUE;
}