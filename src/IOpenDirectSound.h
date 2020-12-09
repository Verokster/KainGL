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

#pragma once

#include "Mmreg.h"
#include "dsound.h"
#include "Allocation.h"

#define STREAM_BUFFER_SIZE 128000

class IOpenDirectSound : public IDirectSound, public Allocation
{
public:
	IOpenDirectSound* last;

	virtual ~IOpenDirectSound() {};

	virtual VOID SetListener(FLOAT*) {};

	// Inherited via IDirectSound
	virtual HRESULT __stdcall QueryInterface(REFIID, LPVOID*);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();
	virtual HRESULT __stdcall CreateSoundBuffer(LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER*, LPUNKNOWN);
	virtual HRESULT __stdcall GetCaps(LPDSCAPS pDSCaps);
	virtual HRESULT __stdcall DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER, LPDIRECTSOUNDBUFFER*);
	virtual HRESULT __stdcall SetCooperativeLevel(HWND, DWORD);
	virtual HRESULT __stdcall Compact();
	virtual HRESULT __stdcall GetSpeakerConfig(LPDWORD);
	virtual HRESULT __stdcall SetSpeakerConfig(DWORD);
	virtual HRESULT __stdcall Initialize(LPCGUID);
};