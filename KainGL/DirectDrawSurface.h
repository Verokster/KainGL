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
#include "DirectDrawPalette.h"
#include "DirectDrawClipper.h"
#include "GLib.h"

class DirectDrawSurface : IDirectDrawSurface
{
public:
	DirectDrawSurface* prev;
	LPDIRECTDRAW ddraw;

	DirectDrawPalette* attachedPallete;
	DirectDrawSurface* attachedSurface;
	DirectDrawClipper* attachedClipper;

	DWORD index;
	BYTE* indexBuffer;

	DirectDrawSurface(LPDIRECTDRAW, BOOL);
	DirectDrawSurface(LPDIRECTDRAW, DirectDrawSurface*, BYTE*);
	~DirectDrawSurface();

	// Inherited via IDirectDrawSurface
	HRESULT __stdcall QueryInterface(REFIID riid, LPVOID * ppvObj);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall AddAttachedSurface(LPDIRECTDRAWSURFACE);
	HRESULT __stdcall AddOverlayDirtyRect(LPRECT);
	HRESULT __stdcall Blt(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
	HRESULT __stdcall BltBatch(LPDDBLTBATCH, DWORD, DWORD);
	HRESULT __stdcall BltFast(DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
	HRESULT __stdcall DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE);
	HRESULT __stdcall EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT __stdcall EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK);
	HRESULT __stdcall Flip(LPDIRECTDRAWSURFACE, DWORD);
	HRESULT __stdcall GetAttachedSurface(LPDDSCAPS, LPDIRECTDRAWSURFACE *);
	HRESULT __stdcall GetBltStatus(DWORD);
	HRESULT __stdcall GetCaps(LPDDSCAPS);
	HRESULT __stdcall GetClipper(LPDIRECTDRAWCLIPPER *);
	HRESULT __stdcall GetColorKey(DWORD, LPDDCOLORKEY);
	HRESULT __stdcall GetDC(HDC *);
	HRESULT __stdcall GetFlipStatus(DWORD);
	HRESULT __stdcall GetOverlayPosition(LPLONG, LPLONG);
	HRESULT __stdcall GetPalette(LPDIRECTDRAWPALETTE *);
	HRESULT __stdcall GetPixelFormat(LPDDPIXELFORMAT);
	HRESULT __stdcall GetSurfaceDesc(LPDDSURFACEDESC);
	HRESULT __stdcall Initialize(LPDIRECTDRAW, LPDDSURFACEDESC);
	HRESULT __stdcall IsLost();
	HRESULT __stdcall Lock(LPRECT, LPDDSURFACEDESC, DWORD, HANDLE);
	HRESULT __stdcall ReleaseDC(HDC);
	HRESULT __stdcall Restore();
	HRESULT __stdcall SetClipper(LPDIRECTDRAWCLIPPER);
	HRESULT __stdcall SetColorKey(DWORD, LPDDCOLORKEY);
	HRESULT __stdcall SetOverlayPosition(LONG, LONG);
	HRESULT __stdcall SetPalette(LPDIRECTDRAWPALETTE);
	HRESULT __stdcall Unlock(LPVOID);
	HRESULT __stdcall UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDOVERLAYFX);
	HRESULT __stdcall UpdateOverlayDisplay(DWORD);
	HRESULT __stdcall UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE);
};

