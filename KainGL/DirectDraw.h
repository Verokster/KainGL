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
#include "ddraw.h"
#include "ExtraTypes.h"
#include "DirectDrawPalette.h"

class DirectDraw : IDirectDraw
{
public:
	DirectDraw* last;
	LPDIRECTDRAWSURFACE surfaceEntries;
	LPDIRECTDRAWPALETTE paletteEntries;
	LPDIRECTDRAWCLIPPER clipperEntries;

	LPDIRECTDRAWSURFACE attachedSurface;

	HWND hWnd;
	HDC hDc;
	HGLRC hRc;
	DisplayMode* virtualMode;
	DisplayMode* realMode;
	BOOL isFinish;
	DWORD mbPressed;

	HANDLE hDrawThread;
	HANDLE hDrawEvent;
	BOOL isDraw;

	Viewport viewport;
	DWORD isStylesLoaded;
	//WindowState windowState;
	BOOL isStateChanged;
	WINDOWPLACEMENT windowPlacement;
	DWORD clearStage;
	DWORD frequency;
	BOOL wasPixelSet;

	DWORD tick;

	DirectDraw(DirectDraw* lastObj);
	~DirectDraw();

	VOID CalcView();
	VOID CheckView();
	HRESULT SetFullscreenMode();
	HRESULT SetWindowedMode();
	VOID ScaleMouseIn(LPARAM* lParam);
	VOID ScaleMouseOut(LPARAM* lParam);
	VOID CheckDisplayMode();

	VOID RenderOld(DWORD glMaxTexSize);
	VOID RenderNew();

	// Inherited via  IDirectDraw
	HRESULT __stdcall QueryInterface(REFIID riid, LPVOID * ppvObj);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall Compact();
	HRESULT __stdcall CreateClipper(DWORD, LPDIRECTDRAWCLIPPER *, IUnknown *);
	HRESULT __stdcall CreatePalette(DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE *, IUnknown *);
	HRESULT __stdcall CreateSurface(LPDDSURFACEDESC, LPDIRECTDRAWSURFACE *, IUnknown *);
	HRESULT __stdcall DuplicateSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE *);
	HRESULT __stdcall EnumDisplayModes(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK);
	HRESULT __stdcall EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT __stdcall FlipToGDISurface();
	HRESULT __stdcall GetCaps(LPDDCAPS, LPDDCAPS);
	HRESULT __stdcall GetDisplayMode(LPDDSURFACEDESC);
	HRESULT __stdcall GetFourCCCodes(LPDWORD, LPDWORD);
	HRESULT __stdcall GetGDISurface(LPDIRECTDRAWSURFACE *);
	HRESULT __stdcall GetMonitorFrequency(LPDWORD);
	HRESULT __stdcall GetScanLine(LPDWORD);
	HRESULT __stdcall GetVerticalBlankStatus(LPBOOL);
	HRESULT __stdcall Initialize(GUID *);
	HRESULT __stdcall RestoreDisplayMode();
	HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	HRESULT __stdcall SetDisplayMode(DWORD, DWORD, DWORD);
	HRESULT __stdcall WaitForVerticalBlank(DWORD, HANDLE);
};