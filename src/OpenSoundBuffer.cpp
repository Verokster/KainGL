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
#include "OpenSoundBuffer.h"
#include "OpenSound.h"
#include "Main.h"
#include "Vibration.h"

HRESULT __stdcall OpenSoundBuffer::QueryInterface(REFIID riid, LPVOID* ppvObj) { return DS_OK; }
ULONG __stdcall OpenSoundBuffer::AddRef() { return 0; }
HRESULT __stdcall OpenSoundBuffer::GetCaps(LPDSBCAPS pDSBufferCaps) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::GetVolume(LPLONG plVolume) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::GetPan(LPLONG plPan) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::GetFrequency(LPDWORD pdwFrequency) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::Initialize(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::SetFormat(LPCWAVEFORMATEX pcfxFormat) { return DS_OK; }
HRESULT __stdcall OpenSoundBuffer::Restore() { return DS_OK; }

OpenSoundBuffer::OpenSoundBuffer(OpenSound* dsound, LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER lpDSBuffer)
{
	MemoryCopy(&this->options, &soundOptions, sizeof(ALSoundOptions));
	MemoryZero(&soundOptions, sizeof(ALSoundOptions));

	this->dsound = dsound;
	this->isManualStopped = TRUE;

	LPWAVEFORMATEX format = pcDSBufferDesc->lpwfxFormat;
	this->frequency = format->nSamplesPerSec;
	if (format->nChannels == 1)
		this->type = format->wBitsPerSample == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	else
		this->type = format->wBitsPerSample == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;

	this->dataSize = pcDSBufferDesc->dwBufferBytes;
	this->data = (BYTE*)MemoryAlloc(this->dataSize);
	MemoryZero(this->data, this->dataSize);

	this->poolIndex = 0;
	this->dataWrote = 0;

	this->isStream = this->dataSize == STREAM_BUFFER_SIZE;
	this->isSubtitled = !this->options.isPositional || this->options.waveIndex == SND_INV;

	ALGenSources(1, &this->source);

	ALSourcef(this->source, AL_GAIN, 1.0f);
	ALSourcei(this->source, AL_SOURCE_RELATIVE, !this->options.isAbsolute);
	ALSourcef(this->source, AL_ROLLOFF_FACTOR, this->options.rolloffFactor);
	ALSourcef(this->source, AL_REFERENCE_DISTANCE, this->options.referenceDistance);
	ALSourcefv(this->source, AL_POSITION, (ALfloat*)&this->options.position);

	MemoryZero(&this->vibration, sizeof(VibrationOptions));

	if (this->isStream)
	{
		ALGenBuffers(STREAM_POOL_SIZE, this->streamPool);
		this->buffer = NULL;
	}
	else if (!this->options.isPositional)
	{
		ALGenBuffers(1, this->streamPool);
		this->buffer = NULL;
	}
	else
	{
		if (ALSource3i)
		{
			switch (this->options.waveIndex)
			{
			case SND_CLICK:
			case SND_HEARTB:
			case SND_INV:
				break;

			case SND_THN1:
			case SND_THN2:
			case SND_THN3:
			case SND_THN4:
			{
				this->vibration.density.left = 0.4f;
				this->vibration.density.right = 0.4f;

				if (this->options.position.x > 0.0)
				{
					if (this->options.position.x < 100.0f)
						this->vibration.density.left *= 1.0f - this->options.position.x / 100.0f;
					else
						this->vibration.density.left = 0.0f;
				}
				else
				{
					if (this->options.position.x > -100.0f)
						this->vibration.density.right *= 1.0f - this->options.position.x / -100.0f;
					else
						this->vibration.density.right = 0.0f;
				}

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
			}
			case SND_HOWL05:
				ALSource3i(this->source, AL_AUXILIARY_SEND_FILTER, dsound->uiEffectSlot[1], 0, NULL);
				if (*isKainInside)
					ALSource3i(this->source, AL_AUXILIARY_SEND_FILTER, dsound->uiEffectSlot[0], 1, NULL);
				break;

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
				if (*isKainInside)
					ALSource3i(this->source, AL_AUXILIARY_SEND_FILTER, dsound->uiEffectSlot[0], 0, NULL);
				break;

			}
		}

		this->buffer = &dsound->buffersPool[this->options.waveIndex];
		if (!this->buffer->id)
			ALGenBuffers(1, (ALuint*)&this->buffer->id);
	}
}

OpenSoundBuffer::~OpenSoundBuffer()
{
	ALSourceStop(this->source);
	ALSourcei(this->source, AL_BUFFER, NULL);

	if (this->isStream)
		ALDeleteBuffers(STREAM_POOL_SIZE, this->streamPool);
	else if (!this->buffer)
		ALDeleteBuffers(1, this->streamPool);

	ALDeleteSources(1, &this->source);

	MemoryFree(this->data);

	Vibration::Remove(&this->vibration);
}

ULONG __stdcall OpenSoundBuffer::Release()
{
	if (this->isSubtitled && activeSoundBuffer == this)
		activeSoundBuffer = NULL;

	delete this;

	return DS_OK;
}

VOID OpenSoundBuffer::CheckPositionalGain()
{
	if (this->vibration.duration && GetTickCount() - this->vibration.start > this->vibration.duration)
		Vibration::Remove(&this->vibration);

	FLOAT value = (FLOAT)*gainVolume / 16384.0f;

	if (*isKainSpeaking)
		value /= 4;

	if (this->options.waveIndex >= SND_THN1 && this->options.waveIndex <= SND_THN4)
	{
		if (*isKainInside)
			value /= 4;
	}

	ALSourcef(this->source, AL_GAIN, value);
}

HRESULT __stdcall OpenSoundBuffer::GetStatus(LPDWORD pdwStatus)
{
	*pdwStatus = this->IsPlaying() || this->isStream && !this->isManualStopped;
	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	DWORD pos;
	ALGetSourcei(this->source, AL_BYTE_OFFSET, (ALint*)&pos);

	if (this->isStream && pos >= STREAM_BUFFER_SIZE)
	{
		ALint processed;
		ALGetSourcei(this->source, AL_BUFFERS_PROCESSED, &processed);

		if (processed)
		{
			ALuint del[STREAM_POOL_SIZE];
			ALSourceUnqueueBuffers(this->source, processed, del);
		}

		pos -= STREAM_BUFFER_SIZE;

		if (!pos && !this->isManualStopped && !this->IsPlaying())
			pos = STREAM_BUFFER_SIZE >> 1;
	}

	*pdwCurrentPlayCursor = pos % this->dataSize;

	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::SetCurrentPosition(DWORD dwNewPosition)
{
	ALSourceStop(this->source);
	ALSourcei(this->source, AL_BUFFER, NULL);
	this->dataWrote = 0;
	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::SetVolume(LONG lVolume)
{
	if (this->options.isPositional)
		this->CheckPositionalGain();
	else
	{
		FLOAT value = lVolume >= -8000 ? 1.0f - (FLOAT)MathLog10((FLOAT)-lVolume) / 4.3f : 0.0f;
		ALSourcef(this->source, AL_GAIN, value);
	}

	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::SetPan(LONG lPan)
{
	ALSourcei(this->source, AL_SOURCE_RELATIVE, !this->options.isAbsolute);
	ALSourcef(this->source, AL_ROLLOFF_FACTOR, this->options.rolloffFactor);
	ALSourcef(this->source, AL_REFERENCE_DISTANCE, this->options.referenceDistance);
	ALSourcefv(this->source, AL_POSITION, (ALfloat*)&this->options.position);
	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::SetFrequency(DWORD dwFrequency)
{
	if (this->options.isPositional &&
		this->options.waveIndex >= SND_THN1 && this->options.waveIndex <= SND_THN4)
	{
		INT r = Random() % 2;
		switch (r)
		{
		case 0:
			this->vibration.density.left /= 2.0f;
			this->vibration.density.right /= 2.0f;
			this->vibration.duration <<= 1;
			ALSourcef(this->source, AL_PITCH, 0.5f);
			break;
		case 2:
			this->vibration.density.left *= 2.0f;
			this->vibration.density.right *= 2.0f;
			this->vibration.duration >>= 1;
			ALSourcef(this->source, AL_PITCH, 2.0f);
			break;
		default:
			ALSourcef(this->source, AL_PITCH, 1.0f);
			break;
		}
	}
	else
		ALSourcef(this->source, AL_PITCH, dwFrequency ? (FLOAT)dwFrequency / this->frequency : 1.0f);

	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	if (this->isSubtitled)
	{
		if (soundSuspendTime)
			soundStartTime += GetTickCount() - soundSuspendTime;

		activeSoundBuffer = this;
	}

	if (!this->dataWrote)
		this->Upload();

	if (!this->isStream)
		ALSourcei(this->source, AL_BUFFER, this->buffer ? this->buffer->id : *this->streamPool);

	ALSourcePlay(this->source);
	this->isManualStopped = FALSE;

	Vibration::Add(&this->vibration);
	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::Stop()
{
	if (this->isSubtitled && activeSoundBuffer == this)
	{
		soundSuspendTime = GetTickCount();
		activeSoundBuffer = NULL;
	}

	ALSourcePause(this->source);
	this->isManualStopped = TRUE;

	Vibration::Remove(&this->vibration);
	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	*ppvAudioPtr1 = this->data;

	if (dwBytes > this->dataSize)
	{
		*pdwAudioBytes1 = this->dataSize;

		if (ppvAudioPtr2)
		{
			*ppvAudioPtr2 = this->data;
			*pdwAudioBytes2 = dwBytes - this->dataSize;
		}
		else
			*pdwAudioBytes2 = 0;
	}
	else
	{
		*pdwAudioBytes1 = dwBytes;
		*pdwAudioBytes2 = 0;
	}

	return DS_OK;
}

HRESULT __stdcall OpenSoundBuffer::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	if (this->dataWrote)
		this->Upload();

	return DS_OK;
}

BOOL OpenSoundBuffer::IsPlaying()
{
	DWORD status;
	ALGetSourcei(this->source, AL_SOURCE_STATE, (ALint*)&status);

	BOOL res = status == AL_PLAYING;
	if (res && this->options.isPositional)
	{
		this->CheckPositionalGain();

		switch (this->options.waveIndex)
		{
		case SND_BLOODUP:
		case SND_GROWL1:
		case SND_GROWL2:
		case SND_KAINHIT:
		case SND_KAINHIT2:
		case SND_KAINHIT3:
		case SND_KLAUGH:
		case SND_VAE:
		case SND_SUCK:
		{
			INT offset = (INT)MathRound(120.0f * 4096 / *scale);

			INT kainX = *((WORD*)*worldObject + 9);
			INT kainY = *((WORD*)*worldObject + 11);

			FLOAT position[3] = {
				(FLOAT)kainX,
				(FLOAT)kainY,
				0.0f
			};

			ALSourcefv(this->source, AL_POSITION, position);
			break;
		}

		default:
			break;
		}
	}

	return res;
}

VOID OpenSoundBuffer::Upload()
{
	if (!this->isStream)
	{
		if (this->buffer)
		{
			if (!this->buffer->loaded)
			{
				this->buffer->loaded = TRUE;
				ALBufferData(this->buffer->id, this->type, this->data, this->dataSize, this->frequency);
				this->dataWrote += this->dataSize;
			}
		}
		else
		{
			ALBufferData(this->streamPool[0], this->type, this->data, this->dataSize, this->frequency);
			this->dataWrote += this->dataSize;
		}
	}
	else
	{
		ALuint buffer = this->streamPool[this->poolIndex++];
		if (this->poolIndex >= STREAM_POOL_SIZE)
			this->poolIndex = 0;

		DWORD pos = (STREAM_BUFFER_SIZE >> 1) * (this->dataWrote / (STREAM_BUFFER_SIZE >> 1) % 2);
		ALBufferData(buffer, this->type, this->data + pos, STREAM_BUFFER_SIZE >> 1, this->frequency);
		this->dataWrote += STREAM_BUFFER_SIZE >> 1;

		ALSourceQueueBuffers(this->source, 1, &buffer);

		if (!this->isManualStopped && !this->IsPlaying())
			this->Play(0, 0, 0);
	}
}