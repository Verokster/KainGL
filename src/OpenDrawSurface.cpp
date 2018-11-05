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
#include "OpenDrawSurface.h"
#include "OpenDraw.h"

#pragma region Not Implemented
HRESULT OpenDrawSurface::QueryInterface(REFIID riid, LPVOID* ppvObj) { return DD_OK; }
ULONG OpenDrawSurface::AddRef() { return 0; }
HRESULT OpenDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT OpenDrawSurface::AddOverlayDirtyRect(LPRECT) { return DD_OK; }
HRESULT OpenDrawSurface::BltBatch(LPDDBLTBATCH, DWORD, DWORD) { return DD_OK; }
HRESULT OpenDrawSurface::BltFast(DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD) { return DD_OK; }
HRESULT OpenDrawSurface::DeleteAttachedSurface(DWORD, LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT OpenDrawSurface::EnumAttachedSurfaces(LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT OpenDrawSurface::EnumOverlayZOrders(DWORD, LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT OpenDrawSurface::GetBltStatus(DWORD) { return DD_OK; }
HRESULT OpenDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER *) { return DD_OK; }
HRESULT OpenDrawSurface::GetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT OpenDrawSurface::GetDC(HDC* hDc) { return DD_OK; }
HRESULT OpenDrawSurface::GetFlipStatus(DWORD) { return DD_OK; }
HRESULT OpenDrawSurface::GetOverlayPosition(LPLONG, LPLONG) { return DD_OK; }
HRESULT OpenDrawSurface::GetPalette(LPDIRECTDRAWPALETTE *) { return DD_OK; }
HRESULT OpenDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC) { return DD_OK; }
HRESULT OpenDrawSurface::Initialize(LPDIRECTDRAW, LPDDSURFACEDESC) { return DD_OK; }
HRESULT OpenDrawSurface::IsLost() { return DD_OK; }
HRESULT OpenDrawSurface::ReleaseDC(HDC) { return DD_OK; }
HRESULT OpenDrawSurface::SetColorKey(DWORD, LPDDCOLORKEY) { return DD_OK; }
HRESULT OpenDrawSurface::SetOverlayPosition(LONG, LONG) { return DD_OK; }
HRESULT OpenDrawSurface::UpdateOverlay(LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDOVERLAYFX) { return DD_OK; }
HRESULT OpenDrawSurface::UpdateOverlayDisplay(DWORD) { return DD_OK; }
HRESULT OpenDrawSurface::UpdateOverlayZOrder(DWORD, LPDIRECTDRAWSURFACE) { return DD_OK; }
HRESULT OpenDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) { return DD_OK; }
HRESULT OpenDrawSurface::Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) { return DD_OK; }
HRESULT OpenDrawSurface::Restore() { return DD_OK; }
#pragma endregion

OpenDrawSurface::OpenDrawSurface(OpenDraw* lpDD, BOOL isDouble)
{
	this->ddraw = lpDD;
	this->prev = this->ddraw->surfaceEntries;
	this->ddraw->surfaceEntries = this;

	this->attachedPallete = NULL;
	this->attachedClipper = NULL;

	this->index = 0;
	this->oldTime = 0.0;

	DisplayMode* dwMode = this->ddraw->virtualMode;
	DWORD bufferSize = dwMode->dwWidth * dwMode->dwHeight * (dwMode->dwBPP >> 3);
	this->indexBuffer = (BYTE*)MemoryAlloc(bufferSize * (isDouble ? 2 : 1));
	this->attachedSurface = isDouble ? new OpenDrawSurface(lpDD, this, this->indexBuffer + bufferSize) : NULL;
}

OpenDrawSurface::OpenDrawSurface(OpenDraw* lpDD, OpenDrawSurface* attached, BYTE* indexBuffer)
{
	this->ddraw = lpDD;
	this->prev = this->ddraw->surfaceEntries;
	this->ddraw->surfaceEntries = this;

	this->attachedPallete = NULL;
	this->attachedClipper = NULL;

	this->index = 1;
	this->oldTime = 0.0;

	this->indexBuffer = indexBuffer;
	this->attachedSurface = attached;
}

OpenDrawSurface::~OpenDrawSurface()
{
	if (!this->index)
		MemoryFree(this->indexBuffer);
}

ULONG OpenDrawSurface::Release()
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
		OpenDrawSurface* entry = this->ddraw->surfaceEntries;
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

HRESULT OpenDrawSurface::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface)
{
	*lplpDDAttachedSurface = this->attachedSurface;
	return DD_OK;
}

HRESULT OpenDrawSurface::GetCaps(LPDDSCAPS lpDDSCaps)
{
	lpDDSCaps->dwCaps = DDSCAPS_MODEX;
	return DD_OK;
}

HRESULT OpenDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	lpDDPixelFormat->dwGBitMask = 0x03E0;
	return DD_OK;
}

HRESULT OpenDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent)
{
	DisplayMode* dwMode = this->ddraw->virtualMode;

	lpDDSurfaceDesc->dwWidth = dwMode->dwWidth;
	lpDDSurfaceDesc->dwHeight = dwMode->dwHeight;
	lpDDSurfaceDesc->lPitch = dwMode->dwWidth * (dwMode->dwBPP >> 3);
	lpDDSurfaceDesc->lpSurface = this->indexBuffer;

	return DD_OK;
}

HRESULT OpenDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	this->attachedClipper = (OpenDrawClipper*)lpDDClipper;
	return DD_OK;
}

HRESULT OpenDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	this->attachedPallete = (OpenDrawPalette*)lpDDPalette;
	return DD_OK;
}

HRESULT OpenDrawSurface::Unlock(LPVOID lpRect)
{
	this->ddraw->attachedSurface = this;
	SetEvent(this->ddraw->hDrawEvent);

	if (!lpRect)
	{
		LONGLONG qp;
		QueryPerformanceFrequency((LARGE_INTEGER*)&qp);
		DOUBLE timerResolution = 0.001 * qp;

		DOUBLE endTime = this->oldTime + configFpsLimit;
		DOUBLE currTime;
		do
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&qp);
			currTime = (DOUBLE)qp / timerResolution;
		} while (currTime < endTime);
		this->oldTime = endTime + configFpsLimit * DWORD((currTime - endTime) / configFpsLimit);
	}

	return DD_OK;
}