/*
	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

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

OpenSound::OpenSound(IOpenDirectSound* last)
{
	this->last = last;
	this->device = ALCOpenDevice(NULL);
	this->context = NULL;
	MemoryZero(this->buffersPool, sizeof(this->buffersPool));
}

OpenSound::~OpenSound()
{
	ALBuffer* buffer = this->buffersPool;

	DWORD count = BUFFERS_POOL_SIZE;
	do
	{
		if (buffer->id)
			ALDeleteBuffers(1, (ALuint*)&buffer->id);

		++buffer;
	} while (--count);

	if (*this->uiEffectSlot)
		ALDeleteAuxiliaryEffectSlots(sizeof(this->uiEffectSlot) / sizeof(ALuint), this->uiEffectSlot);

	if (*this->uiEffect)
		ALDeleteEffects(sizeof(this->uiEffect) / sizeof(ALuint), this->uiEffect);

	ALCMakeContextCurrent(NULL);
	ALCDestroyContext(this->context);
	ALCCloseDevice(this->device);
}

HRESULT OpenSound::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	*(OpenSoundBuffer**)ppDSBuffer = new OpenSoundBuffer(this, pcDSBufferDesc, *ppDSBuffer);
	return DS_OK;
}

HRESULT OpenSound::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
	if (!this->device)
		return DSERR_INVALIDPARAM;

	if (this->context)
	{
		ALCMakeContextCurrent(NULL);
		ALCDestroyContext(this->context);
	}

	this->context = ALCCreateContext(this->device, NULL);
	if (!this->context)
		return DSERR_INVALIDPARAM;

	ALCMakeContextCurrent(this->context);
	ALDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
	

	if (AL::ContextLoad())
	{
		ALListenerf(AL_METERS_PER_UNIT, 0.032f);

		*this->uiEffectSlot = 0;
		ALGenAuxiliaryEffectSlots(sizeof(this->uiEffectSlot) / sizeof(ALuint), this->uiEffectSlot);

		*this->uiEffect = 0;
		ALGenEffects(sizeof(this->uiEffect) / sizeof(ALuint), this->uiEffect);

		// reverb inside
		{
			ALEffecti(this->uiEffect[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
			AL::UpdateEAXReverb(this->uiEffect[0], &EFX_REVERB_CASTLE_SHORTPASSAGE);
			ALAuxiliaryEffectSloti(this->uiEffectSlot[0], AL_EFFECTSLOT_EFFECT, this->uiEffect[0]);
		}

		// reverb outdoor
		{
			ALEffecti(this->uiEffect[1], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
			AL::UpdateEAXReverb(this->uiEffect[1], &EFX_REVERB_OUTDOORS_DEEPCANYON);
			ALAuxiliaryEffectSloti(this->uiEffectSlot[1], AL_EFFECTSLOT_EFFECT, this->uiEffect[1]);
		}
	}

	return DS_OK;
}

VOID OpenSound::SetListener(FLOAT* point)
{
	ALListenerfv(AL_POSITION, point);
}
