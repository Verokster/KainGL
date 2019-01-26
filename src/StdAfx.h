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

#pragma once
#define WIN32_LEAN_AND_MEAN

#include "windows.h"
#include "stdlib.h"
#include "stdio.h"
#include "ExtraTypes.h"
#include "Mmreg.h"
#include "dsound.h"
#include "xinput.h"

#define _USE_MATH_DEFINES

#define WC_DRAW "f02b3562-1990-42cd-a07f-c2e1c367facd"
#define WIN_STYLE (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS)
#define FS_STYLE (WS_POPUP | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS)

#define SND_BATS 0
#define SND_BLOODUP 1
#define SND_BURN 2
#define SND_CHOIR2 3
#define SND_CLICK 4
#define SND_DEMR 5
#define SND_EXPLODE 6
#define SND_FHIT1 7
#define SND_FHIT2 8
#define SND_FLAMESW 9
#define SND_FLAMESW2 10
#define SND_FLAY 11
#define SND_HEARTB 12
#define SND_MACE 13
#define SND_MALDEATH 14
#define SND_MDEATH 15
#define SND_MDEATH2 16
#define SND_MISSILE 17
#define SND_PYRAMID 18
#define SND_SOUL 19
#define SND_SUCK 20
#define SND_SWING 21
#define SND_SWORD2 22
#define SND_SWORD3 23
#define SND_SWORD4 24
#define SND_TELE 25
#define SND_UNLOCK 26
#define SND_VAE 27
#define SND_ZOMBIE2 28
#define SND_MALEHIT1 29
#define SND_MALEHIT2 30
#define SND_MONHIT1 31
#define SND_KAINHIT 32
#define SND_KAINHIT2 33
#define SND_KAINHIT3 34
#define SND_KLAUGH 35
#define SND_FTORCH1 36
#define SND_TORCH1 37
#define SND_TORCH2 38
#define SND_SNOR05 39
#define SND_GROWL1 40
#define SND_GROWL2 41
#define SND_HOWL05 42
#define SND_SKEY2 43
#define SND_THN1 44
#define SND_THN2 45
#define SND_THN3 46
#define SND_THN4 47
#define SND_INV 48
#define SND_FLASH SND_THN1

#define DIR_TOP 0;
#define DIR_TOP_RIGHT 1;
#define DIR_RIGHT 2;
#define DIR_BOTTOM_RIGHT 3;
#define DIR_BOTTOM 4;
#define DIR_BOTTOM_LEFT 5;
#define DIR_LEFT 6;
#define DIR_TOP_LEFT 7;
#define DIR_NONE 9;

extern HMODULE hDllModule;
extern HANDLE hActCtx;

typedef HANDLE(__stdcall *CREATEACTCTXA)(ACTCTX* pActCtx);
typedef VOID(__stdcall *RELEASEACTCTX)(HANDLE hActCtx);
typedef BOOL(__stdcall *ACTIVATEACTCTX)(HANDLE hActCtx, ULONG_PTR* lpCookie);
typedef BOOL(__stdcall *DEACTIVATEACTCTX)(DWORD dwFlags, ULONG_PTR ulCookie);

extern CREATEACTCTXA CreateActCtxC;
extern RELEASEACTCTX ReleaseActCtxC;
extern ACTIVATEACTCTX ActivateActCtxC;
extern DEACTIVATEACTCTX DeactivateActCtxC;

typedef INT(__stdcall *DRAWTEXTW)(HDC hdc, LPCWSTR lpchText, INT cchText, LPRECT lprc, UINT format);
extern DRAWTEXTW DrawTextUni;

typedef HRESULT(__stdcall *DWMISCOMPOSITIONENABLED)(BOOL* pfEnabled);
extern DWMISCOMPOSITIONENABLED IsDwmCompositionEnabled;

typedef INT(__stdcall *ADDFONTRESOURCEEXA)(LPCSTR name, DWORD fl, PVOID res);
typedef BOOL(__stdcall *REMOVEFONTRESOURCEEXA)(LPCSTR name, DWORD fl, PVOID pdv);

extern ADDFONTRESOURCEEXA AddFontResourceC;
extern REMOVEFONTRESOURCEEXA RemoveFontResourceC;

typedef VOID*(__cdecl *MALLOC)(size_t);
typedef VOID(__cdecl *FREE)(VOID*);
typedef VOID*(__cdecl *MEMSET)(VOID*, INT, size_t);
typedef VOID*(__cdecl *MEMCPY)(VOID*, const VOID*, size_t);
typedef INT(__cdecl *MEMCMP)(const VOID*, const VOID*, size_t);
typedef DOUBLE(__cdecl *CEIL)(DOUBLE);
typedef DOUBLE(__cdecl *FLOOR)(DOUBLE);
typedef DOUBLE(__cdecl *ROUND)(DOUBLE);
typedef DOUBLE(__cdecl *LOG10)(DOUBLE);
typedef DOUBLE(__cdecl *SQRT)(DOUBLE);
typedef DOUBLE(__cdecl *ATAN2)(DOUBLE, DOUBLE);
typedef INT(__cdecl *SPRINTF)(CHAR*, const CHAR*, ...);
typedef CHAR*(__cdecl *STRSTR)(const CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCHR)(const CHAR*, INT);
typedef INT(__cdecl *STRCMP)(const CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCPY)(CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRCAT)(CHAR*, const CHAR*);
typedef CHAR*(__cdecl *STRDUP)(const CHAR*);
typedef size_t(__cdecl *STRLEN)(const CHAR*);
typedef CHAR*(__cdecl *STRRCHR)(const CHAR*, INT);
typedef FILE*(__cdecl *FOPEN)(const CHAR*, const CHAR*);
typedef INT(__cdecl *FCLOSE)(FILE*);
typedef size_t(__cdecl *FREAD)(VOID*, size_t, size_t, FILE*);
typedef size_t(__cdecl *FWRITE)(const VOID*, size_t, size_t, FILE*);
typedef INT(__cdecl *FSEEK)(FILE*, LONG, INT);
typedef INT(__cdecl *RAND)();
typedef VOID(__cdecl *SRAND)(DWORD);
typedef VOID(__cdecl *EXIT)(INT);

extern MALLOC MemoryAlloc;
extern FREE MemoryFree;
extern MEMSET MemorySet;
extern MEMCPY MemoryCopy;
extern MEMCMP MemoryCompare;
extern CEIL MathCeil;
extern FLOOR MathFloor;
extern ROUND MathRound;
extern LOG10 MathLog10;
extern SQRT MathSqrt;
extern ATAN2 MathAtan2;
extern SPRINTF StrPrint;
extern STRSTR StrStr;
extern STRCHR StrChar;
extern STRCMP StrCompare;
extern STRCPY StrCopy;
extern STRCAT StrCat;
extern STRDUP StrDuplicate;
extern STRLEN StrLength;
extern STRRCHR StrRightChar;
extern FOPEN FileOpen;
extern FCLOSE FileClose;
extern FREAD FileRead;
extern FWRITE FileWrite;
extern FSEEK FileSeek;
extern RAND Random;
extern SRAND SeedRandom;
extern EXIT Exit;

#define MemoryZero(Destination,Length) MemorySet((Destination),0,(Length))

typedef HRESULT(__stdcall *DIRECTSOUNDCREATE)(LPCGUID lpcGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);
extern DIRECTSOUNDCREATE DSCreate;

typedef DWORD(__stdcall *XINPUTGETSTATE)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD(__stdcall *XINPUTSETSTATE)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
typedef DWORD(__stdcall *XINPUTGETCAPABILITIES)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);

extern XINPUTGETSTATE InputGetState;
extern XINPUTSETSTATE InputSetState;
extern XINPUTGETCAPABILITIES InputGetCapabilities;

extern LPARAM mousePos;
extern HWND mousehWnd;

extern FLOAT currentTimeout;

VOID LoadKernel32();
VOID LoadGdi32();
VOID LoadUnicoWS();
VOID LoadDwmAPI();
VOID LoadMsvCRT();
VOID LoadDDraw();
VOID LoadXInput();

WCHAR* __fastcall DecodeUtf8(BYTE* ptr, DWORD* count);
WCHAR* __fastcall DecodeUtf8(BYTE* ptr, DWORD* count, DWORD* length);

extern IDirectSoundBuffer* activeSoundBuffer;
extern DWORD soundStartTime;
extern DWORD soundSuspendTime;

extern SubtitlesItem* subtitlesList;
extern SubtitlesItem* subtitlesCurrent;
extern DWORD subtitlesType;
extern SubtitlesFont subtitlesFonts[];
extern CHAR fontFiles[][MAX_PATH];

extern DWORD* scale;
extern ALSoundOptions soundOptions;

extern BOOL* xJoyListConnected;
extern BOOL xJoyListCheck[];
extern INT* gainVolume;
extern bool* isKainInside;
extern BOOL* isKainSpeaking;
extern DWORD* worldObject;

extern struct ModePtr
{
	BOOL* hiRes;
	BOOL* hiColor;
	BOOL* window;
	BOOL* interlaced;
	BOOL* upscale;
} flags;