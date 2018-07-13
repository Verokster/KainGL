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

#pragma once
#define WIN32_LEAN_AND_MEAN

#include "windows.h"
#include "stdlib.h"
#include "stdio.h"

#if (_WIN32_WINNT <= 0x0501)
#define MAX_LINKID_TEXT     48
#define L_MAX_URL_LENGTH    (2048 + 32 + sizeof("://"))
#define ACTCTX_FLAG_RESOURCE_NAME_VALID             (0x00000008)
#define ACTCTX_FLAG_HMODULE_VALID                   (0x00000080)

typedef struct tagLITEM
{
	UINT        mask;
	int         iLink;
	UINT        state;
	UINT        stateMask;
	WCHAR       szID[MAX_LINKID_TEXT];
	WCHAR       szUrl[L_MAX_URL_LENGTH];
} LITEM, *PLITEM;

typedef struct tagNMLINK
{
	NMHDR       hdr;
	LITEM     item;
} NMLINK, *PNMLINK;

typedef struct ACTCTXC {
	ULONG       cbSize;
	DWORD       dwFlags;
	LPCSTR      lpSource;
	USHORT      wProcessorArchitecture;
	LANGID      wLangId;
	LPCSTR      lpAssemblyDirectory;
	LPCSTR      lpResourceName;
	LPCSTR      lpApplicationName;
	HMODULE     hModule;
} ACTCTXA, *PACTCTXA;
typedef const ACTCTXA *PCACTCTXA;
typedef ACTCTXA ACTCTX;
#endif

typedef HANDLE(__stdcall *CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall *RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall *ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall *DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern HMODULE hDllModule;
extern HANDLE hActCtx;

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef VOID*(__cdecl *MALLOC)(size_t);
typedef VOID(__cdecl *FREE)(VOID*);
typedef VOID*(__cdecl *MEMSET)(VOID*, INT, size_t);
typedef VOID*(__cdecl *MEMCPY)(VOID*, const VOID*, size_t);
typedef double(__cdecl *CEIL)(double);
typedef double(__cdecl *FLOOR)(double);
typedef double(__cdecl *ROUND)(double);
typedef INT(__cdecl *SPRINTF)(CHAR*, const CHAR*, ...);
typedef CHAR*(__cdecl *STRSTR)(const CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCPY)(CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCAT)(CHAR*, const CHAR*);
typedef FILE*(__cdecl *FOPEN)(const CHAR*, const CHAR*);
typedef INT(__cdecl *FCLOSE)(FILE*);
typedef VOID(__cdecl *EXIT)(INT);

extern MALLOC MemoryAlloc;
extern FREE MemoryFree;
extern MEMSET MemorySet;
extern MEMCPY MemoryCopy;
extern CEIL MathCeil;
extern FLOOR MathFloor;
extern ROUND MathRound;
extern SPRINTF StrPrint;
extern STRSTR StrStr;
extern STRCPY StrCopy;
extern STRCAT StrCat;
extern FOPEN FileOpen;
extern FCLOSE FileClose;
extern EXIT Exit;

#define MemoryZero(Destination,Length) MemorySet((Destination),0,(Length))

extern LPARAM mousePos;
extern HWND mousehWnd;

extern FLOAT currentTimeout;

VOID LoadKernel32();
VOID LoadMsvCRT();
VOID LoadDDraw();