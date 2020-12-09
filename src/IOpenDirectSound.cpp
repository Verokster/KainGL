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
#include "IOpenDirectSound.h"
#include "Main.h"

HRESULT __stdcall IOpenDirectSound::QueryInterface(REFIID, LPVOID *) { return DS_OK; }
ULONG __stdcall IOpenDirectSound::AddRef() { return 0; }
HRESULT __stdcall IOpenDirectSound::CreateSoundBuffer(LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER *, LPUNKNOWN) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::GetCaps(LPDSCAPS) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER, LPDIRECTSOUNDBUFFER *) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::SetCooperativeLevel(HWND, DWORD) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::Compact() { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::GetSpeakerConfig(LPDWORD) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::SetSpeakerConfig(DWORD) { return DS_OK; }
HRESULT __stdcall IOpenDirectSound::Initialize(LPCGUID) { return DS_OK; }

ULONG __stdcall IOpenDirectSound::Release()
{
	if (dsoundList == this)
		dsoundList = NULL;
	else
	{
		IOpenDirectSound* dsound = dsoundList;
		while (dsound)
		{
			if (dsound->last == this)
			{
				dsound->last = this->last;
				break;
			}

			dsound = dsound->last;
		}
	}

	delete this;
	return DS_OK;
}