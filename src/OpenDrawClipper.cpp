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

#include "stdafx.h"
#include "OpenDrawClipper.h"
#include "OpenDraw.h"

#pragma region Not Implemented
HRESULT OpenDrawClipper::QueryInterface(REFIID riid, LPVOID * ppvObj) { return DD_OK; }
ULONG OpenDrawClipper::AddRef() { return 0; }
HRESULT OpenDrawClipper::GetClipList(LPRECT, LPRGNDATA, LPDWORD) { return DD_OK; }
HRESULT OpenDrawClipper::GetHWnd(HWND *) { return DD_OK; }
HRESULT OpenDrawClipper::Initialize(LPDIRECTDRAW, DWORD) { return DD_OK; }
HRESULT OpenDrawClipper::IsClipListChanged(BOOL *) { return DD_OK; }
HRESULT OpenDrawClipper::SetClipList(LPRGNDATA, DWORD) { return DD_OK; }
HRESULT OpenDrawClipper::SetHWnd(DWORD dwFlags, HWND hWnd) { return DD_OK; }
#pragma endregion

OpenDrawClipper::OpenDrawClipper(OpenDraw* lpDD)
{
	this->ddraw = lpDD;
	this->prev = this->ddraw->clipperEntries;
}

ULONG OpenDrawClipper::Release()
{
	if (this->ddraw->clipperEntries == this)
		this->ddraw->clipperEntries = NULL;
	else
	{
		OpenDrawClipper* entry = this->ddraw->clipperEntries;
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