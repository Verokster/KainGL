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
#include "GLib.h"
#include "Config.h"
#include "DirectDraw.h"

#pragma region Not Implemented
HRESULT DirectDrawSurface::QueryInterface(REFIID riid, LPVOID* ppvObj) { return DD_OK; }
ULONG DirectDrawSurface::AddRef() { return 0; }
HRESULT DirectDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT DirectDrawSurface::AddOverlayDirtyRect(LPRECT) { return DD_OK; }
HRESULT DirectDrawSurface::BltBatch(LPDDBLTBATCH, DWORD, DWORD) { return DD_OK; }
HRESULT DirectDrawSurface::BltFast(DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD) { return DD_OK; }
HRESULT DirectDrawSurface::DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT DirectDrawSurface::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT DirectDrawSurface::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT DirectDrawSurface::GetBltStatus(DWORD) { return DD_OK; }
HRESULT DirectDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER *) { return DD_OK; }
HRESULT DirectDrawSurface::GetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT DirectDrawSurface::GetDC(HDC* hDc) { return DD_OK; }
HRESULT DirectDrawSurface::GetFlipStatus(DWORD) { return DD_OK; }
HRESULT DirectDrawSurface::GetOverlayPosition(LPLONG, LPLONG) { return DD_OK; }
HRESULT DirectDrawSurface::GetPalette(LPDIRECTDRAWPALETTE *) { return DD_OK; }
HRESULT DirectDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC) { return DD_OK; }
HRESULT DirectDrawSurface::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC) { return DD_OK; }
HRESULT DirectDrawSurface::IsLost() { return DD_OK; }
HRESULT DirectDrawSurface::ReleaseDC(HDC) { return DD_OK; }
HRESULT DirectDrawSurface::SetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT DirectDrawSurface::SetOverlayPosition(LONG, LONG) { return DD_OK; }
HRESULT DirectDrawSurface::UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDOVERLAYFX) { return DD_OK; }
HRESULT DirectDrawSurface::UpdateOverlayDisplay(DWORD) { return DD_OK; }
HRESULT DirectDrawSurface::UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT DirectDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) { return DD_OK; }
HRESULT DirectDrawSurface::Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) { return DD_OK; }
HRESULT DirectDrawSurface::Restore() { return DD_OK; }
#pragma endregion

DirectDrawSurface::DirectDrawSurface(DirectDraw* lpDD, BOOL isDouble)
{
	this->ddraw = lpDD;
	this->prev = this->ddraw->surfaceEntries;
	this->ddraw->surfaceEntries = this;

	this->attachedPallete = NULL;
	this->attachedClipper = NULL;

	this->index = 0;

	DisplayMode* dwMode = this->ddraw->virtualMode;
	DWORD bufferSize = dwMode->dwWidth * dwMode->dwHeight * (dwMode->dwBPP >> 3);
	this->indexBuffer = (BYTE*)MemoryAlloc(bufferSize * (isDouble ? 2 : 1));
	this->attachedSurface = isDouble ? new DirectDrawSurface(lpDD, this, this->indexBuffer + bufferSize) : NULL;
}

DirectDrawSurface::DirectDrawSurface(DirectDraw* lpDD, DirectDrawSurface* attached, BYTE* indexBuffer)
{
	this->ddraw = lpDD;
	this->prev = this->ddraw->surfaceEntries;
	this->ddraw->surfaceEntries = this;

	this->attachedPallete = NULL;
	this->attachedClipper = NULL;

	this->index = 1;

	this->indexBuffer = indexBuffer;
	this->attachedSurface = attached;
}

DirectDrawSurface::~DirectDrawSurface()
{
	if (!this->index)
		MemoryFree(this->indexBuffer);
}

ULONG DirectDrawSurface::Release()
{
	if (this->index == 0)
	{
		this->ddraw->RenderStop();
		this->ddraw->attachedSurface = NULL;
		this->attachedSurface->Release();
	}

	if (this->ddraw->surfaceEntries == this)
		this->ddraw->surfaceEntries = NULL;
	else
	{
		DirectDrawSurface* entry = this->ddraw->surfaceEntries;
		while (entry)
		{
			if (entry->prev == this)
			{
				entry->prev = this->prev;
				break;
			}

			entry = entry->prev;
		}
	}

	delete this;
	return 0;
}

HRESULT DirectDrawSurface::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface)
{
	*lplpDDAttachedSurface = this->attachedSurface;
	return DD_OK;
}

HRESULT DirectDrawSurface::GetCaps(LPDDSCAPS lpDDSCaps)
{
	lpDDSCaps->dwCaps = DDSCAPS_MODEX;
	return DD_OK;
}

HRESULT DirectDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwGBitMask = 0x03E0;
	return DD_OK;
}

HRESULT DirectDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	DisplayMode* dwMode = this->ddraw->virtualMode;

	lpDDSurfaceDesc->dwWidth = dwMode->dwWidth;
	lpDDSurfaceDesc->dwHeight = dwMode->dwHeight;
	lpDDSurfaceDesc->lPitch = dwMode->dwWidth * (dwMode->dwBPP >> 3);
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	return DD_OK;
}

HRESULT DirectDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	this->attachedClipper = (DirectDrawClipper*)lpDDClipper;
	return DD_OK;
}

HRESULT DirectDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	this->attachedPallete = (DirectDrawPalette*)lpDDPalette;
	return DD_OK;
}

DOUBLE oldTime;
HRESULT DirectDrawSurface::Unlock(LPVOID lpRect)
{
	this->ddraw->attachedSurface = this;
 	SetEvent(this->ddraw->hDrawEvent);
	
	LONGLONG qpf;
	QueryPerformanceFrequency((LARGE_INTEGER*)&qpf);
	DOUBLE timerResolution = 0.001 * qpf;

	DOUBLE endTime = oldTime + configFpsLimit;
	DOUBLE currentTime = 0;
	do
	{
		LONGLONG qpc;
		QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		currentTime = (DOUBLE)qpc / timerResolution;
	} while (currentTime < endTime);
	oldTime = currentTime;

	return DD_OK;
}