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
#include "DirectSound.h"
#include "DirectSoundBuffer.h"

DirectSound::DirectSound(IOpenDirectSound* last, LPDIRECTSOUND lpDS)
{
	this->last = last;
	this->ds = lpDS;
}

HRESULT DirectSound::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	return this->ds->QueryInterface(riid, ppvObj);
}

ULONG DirectSound::AddRef()
{
	return this->ds->AddRef();
}

ULONG DirectSound::Release()
{
	ULONG res = this->ds->Release();
	IOpenDirectSound::Release();
	return res;
}

HRESULT DirectSound::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	HRESULT res = this->ds->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);
	if (res == DS_OK)
		*(DirectSoundBuffer**)ppDSBuffer = new DirectSoundBuffer(*ppDSBuffer);
	return res;
}

HRESULT DirectSound::GetCaps(LPDSCAPS pDSCaps)
{
	return this->ds->GetCaps(pDSCaps);
}

HRESULT DirectSound::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER* ppDSBufferDuplicate)
{
	return this->ds->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);
}

HRESULT DirectSound::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
	return this->ds->SetCooperativeLevel(hwnd, dwLevel);
}

HRESULT DirectSound::Compact()
{
	return this->ds->Compact();
}

HRESULT DirectSound::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
	return this->ds->GetSpeakerConfig(pdwSpeakerConfig);
}

HRESULT DirectSound::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
	return this->ds->SetSpeakerConfig(dwSpeakerConfig);
}

HRESULT DirectSound::Initialize(LPCGUID pcGuidDevice)
{
	return this->ds->Initialize(pcGuidDevice);
}
