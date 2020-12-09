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
#include "DirectSoundBuffer.h"
#include "Vibration.h"
#include "Main.h"

DirectSoundBuffer::DirectSoundBuffer(LPDIRECTSOUNDBUFFER lpDSBuffer)
{
	MemoryCopy(&this->options, &soundOptions, sizeof(ALSoundOptions));
	MemoryZero(&soundOptions, sizeof(ALSoundOptions));

	this->dsBuffer = lpDSBuffer;

	MemoryZero(&this->vibration, sizeof(VibrationOptions));
	if (this->options.isPositional)
	{
		switch (this->options.waveIndex)
		{
		case SND_THN1:
		case SND_THN2:
		case SND_THN3:
		case SND_THN4:
		{
			this->vibration.density.left = 0.4f;
			this->vibration.density.right = 0.4f;
			this->vibration.duration = 1000;
			if (*isKainSpeaking)
			{
				this->vibration.density.left /= 2.0f;
				this->vibration.density.right /= 2.0f;
			}

			if (*isKainInside)
			{
				this->vibration.density.left /= 2.0f;
				this->vibration.density.right /= 2.0f;
			}

			break;
		}
		case SND_GROWL1:
		case SND_GROWL2:
		case SND_KAINHIT:
		case SND_KAINHIT2:
		case SND_KAINHIT3:
		{
			this->vibration.density.left = 0.5f;
			this->vibration.density.right = 0.5f;
			this->vibration.duration = 1000;
			if (*isKainSpeaking)
			{
				this->vibration.density.left /= 2.0f;
				this->vibration.density.right /= 2.0f;
			}

			break;
		}

		default:
			break;
		}
	}

	this->isSubtitled = !this->options.isPositional || this->options.waveIndex == SND_INV;
}

DirectSoundBuffer::~DirectSoundBuffer()
{
	Vibration::Remove(&this->vibration);
}

HRESULT __stdcall DirectSoundBuffer::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	return this->dsBuffer->QueryInterface(riid, ppvObj);
}

ULONG __stdcall DirectSoundBuffer::AddRef()
{
	return this->dsBuffer->AddRef();
}

ULONG __stdcall DirectSoundBuffer::Release()
{
	if (this->isSubtitled && activeSoundBuffer == this)
	{
		activeSoundBuffer = NULL;
		Main::SetSyncDraw();
	}

	ULONG res = this->dsBuffer->Release();
	delete this;
	return res;
}

HRESULT __stdcall DirectSoundBuffer::GetCaps(LPDSBCAPS pDSBufferCaps)
{
	return this->dsBuffer->GetCaps(pDSBufferCaps);
}

HRESULT __stdcall DirectSoundBuffer::GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	return this->dsBuffer->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);
}

HRESULT __stdcall DirectSoundBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
	return this->dsBuffer->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT __stdcall DirectSoundBuffer::GetVolume(LPLONG plVolume)
{
	return this->dsBuffer->GetVolume(plVolume);
}

HRESULT __stdcall DirectSoundBuffer::GetPan(LPLONG plPan)
{
	return this->dsBuffer->GetPan(plPan);
}

HRESULT __stdcall DirectSoundBuffer::GetFrequency(LPDWORD pdwFrequency)
{
	return this->dsBuffer->GetFrequency(pdwFrequency);
}

HRESULT __stdcall DirectSoundBuffer::GetStatus(LPDWORD pdwStatus)
{
	if (this->vibration.duration && timeGetTime() - this->vibration.start > this->vibration.duration)
		Vibration::Remove(&this->vibration);

	if (this->isSubtitled && activeSoundBuffer == this && subtitlesCurrent)
	{
		DWORD nextSync = this->lastSyncTime + 66;
		DOUBLE currTime = timeGetTime();
		if (currTime >= nextSync)
		{
			this->lastSyncTime = nextSync;
			Main::SetSyncDraw();
		}
	}

	return this->dsBuffer->GetStatus(pdwStatus);
}

HRESULT __stdcall DirectSoundBuffer::Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc)
{
	return this->dsBuffer->Initialize(pDirectSound, pcDSBufferDesc);
}

HRESULT __stdcall DirectSoundBuffer::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	return this->dsBuffer->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT __stdcall DirectSoundBuffer::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	if (this->isSubtitled)
	{
		if (soundSuspendTime)
			soundStartTime += timeGetTime() - soundSuspendTime;

		activeSoundBuffer = this;
		Main::SetSyncDraw();
		this->lastSyncTime = timeGetTime();
	}

	Vibration::Add(&this->vibration);

	return this->dsBuffer->Play(dwReserved1, dwPriority, dwFlags);;
}

HRESULT __stdcall DirectSoundBuffer::SetCurrentPosition(DWORD dwNewPosition)
{
	return this->dsBuffer->SetCurrentPosition(dwNewPosition);
}

HRESULT __stdcall DirectSoundBuffer::SetFormat(LPCWAVEFORMATEX pcfxFormat)
{
	return this->dsBuffer->SetFormat(pcfxFormat);
}

HRESULT __stdcall DirectSoundBuffer::SetVolume(LONG lVolume)
{
	return this->dsBuffer->SetVolume(lVolume);
}

HRESULT __stdcall DirectSoundBuffer::SetPan(LONG lPan)
{
	return this->dsBuffer->SetPan(lPan);
}

HRESULT __stdcall DirectSoundBuffer::SetFrequency(DWORD dwFrequency)
{
	return this->dsBuffer->SetFrequency(dwFrequency);
}

HRESULT __stdcall DirectSoundBuffer::Stop()
{
	if (this->isSubtitled && activeSoundBuffer == this)
	{
		soundSuspendTime = timeGetTime();
		activeSoundBuffer = NULL;
		Main::SetSyncDraw();
	}

	Vibration::Remove(&this->vibration);

	return this->dsBuffer->Stop();
}

HRESULT __stdcall DirectSoundBuffer::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	return this->dsBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}

HRESULT __stdcall DirectSoundBuffer::Restore()
{
	return this->dsBuffer->Restore();
}
