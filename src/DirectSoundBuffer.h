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

class DirectSoundBuffer : IDirectSoundBuffer, public Allocation
{
private:
	LPDIRECTSOUNDBUFFER dsBuffer;
	BOOL isSubtitled;
	ALSoundOptions options;
	VibrationOptions vibration;
	DWORD lastSyncTime;

public:
	DirectSoundBuffer(LPDIRECTSOUNDBUFFER);
	~DirectSoundBuffer();

	// Inherited via IDirectSoundBuffer
	HRESULT __stdcall QueryInterface(REFIID, LPVOID*);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall GetCaps(LPDSBCAPS);
	HRESULT __stdcall GetCurrentPosition(LPDWORD, LPDWORD);
	HRESULT __stdcall GetFormat(LPWAVEFORMATEX, DWORD, LPDWORD);
	HRESULT __stdcall GetVolume(LPLONG);
	HRESULT __stdcall GetPan(LPLONG);
	HRESULT __stdcall GetFrequency(LPDWORD);
	HRESULT __stdcall GetStatus(LPDWORD);
	HRESULT __stdcall Initialize(LPDIRECTSOUND, LPCDSBUFFERDESC);
	HRESULT __stdcall Lock(DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD);
	HRESULT __stdcall Play(DWORD, DWORD, DWORD);
	HRESULT __stdcall SetCurrentPosition(DWORD);
	HRESULT __stdcall SetFormat(LPCWAVEFORMATEX);
	HRESULT __stdcall SetVolume(LONG);
	HRESULT __stdcall SetPan(LONG);
	HRESULT __stdcall SetFrequency(DWORD);
	HRESULT __stdcall Stop();
	HRESULT __stdcall Unlock(LPVOID, DWORD, LPVOID, DWORD);
	HRESULT __stdcall Restore();
};

