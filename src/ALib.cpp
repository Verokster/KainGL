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
#include "ALib.h"

const EFXEAXREVERBPROPERTIES EFX_REVERB_CASTLE_SHORTPASSAGE = { 1.0f, 0.889999986f, 0.27f, 0.316227764f, 0.100000001f, 2.31999993f, 0.829999983f, 0.310000002f, 0.891250908f, 0.00700000022f, {0.0f, 0.0f, 0.0f}, 1.25892544f, 0.023f, {0.0f, 0.0f, 0.0f}, 0.137999997f, 0.0799999982f, 0.25f, 0.0f, 0.994260073f, 5168.6001f, 139.5f, 0.0f, 1 };
//const EFXEAXREVERBPROPERTIES EFX_REVERB_OUTDOORS_VALLEY = { 1.0f, 0.280000001f, 0.1f, 0.0281838328f, 0.158489317f, 2.88000011f, 0.25999999f, 0.349999994f, 0.14125374f, 0.263000011f, {0.0f, 0.0f, -0.0f}, 0.398107171f, 0.100000001f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.340000004f, 0.25f, 0.0f, 0.994260073f, 2854.3999f, 107.5f, 0.0f, 0 };
const EFXEAXREVERBPROPERTIES EFX_REVERB_OUTDOORS_DEEPCANYON = { 1.0f, 0.74000001f, 1.0f, 0.27827939f, 1.0f, 3.8900001f, 0.209999993f, 0.460000008f, 0.316227764f, 0.223000005f, {0.0f, 0.0f, -0.0f}, 0.354813397f, 0.0189999994f, {0.0f, 0.0f, 0.0f}, 0.25f, 1.0f, 0.25f, 0.0f, 0.994260073f, 4399.1001f, 242.899994f, 0.0f, 0 };

#ifdef _DEBUG
ALGETERROR ALGetError;
#endif

ALCOPENDEVICE ALCOpenDevice;
ALCCLOSEDEVICE ALCCloseDevice;
ALCCREATECONTEXT ALCCreateContext;
ALCDESTROYCONTEXT ALCDestroyContext;
ALCMAKECONTEXTCURRENT ALCMakeContextCurrent;

ALDISTANCEMODEL ALDistanceModel;
ALLISTENERFV ALListenerfv;
ALLISTENERF ALListenerf;

ALGENSOURCES ALGenSources;
ALDELETESOURCES ALDeleteSources;
ALGENBUFFERS ALGenBuffers;
ALDELETEBUFFERS ALDeleteBuffers;

ALBUFFERDATA ALBufferData;
ALSOURCEQUEUEBUFFERS ALSourceQueueBuffers;
ALSOURCEUNQUEUEBUFFERS ALSourceUnqueueBuffers;

ALGETSOURCEI ALGetSourcei;
ALSOURCEI ALSourcei;
ALSOURCEF ALSourcef;
ALSOURCEFV ALSourcefv;
ALSOURCE3I ALSource3i;

ALSOURCEPLAY ALSourcePlay;
ALSOURCESTOP ALSourceStop;
ALSOURCEPAUSE ALSourcePause;

ALCISEXTENSIONPRESENT ALCIsExtensionPresent;
ALGETPROCADDRESS ALGetProcAddress;

LPALGENAUXILIARYEFFECTSLOTS ALGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS ALDeleteAuxiliaryEffectSlots;
LPALAUXILIARYEFFECTSLOTI ALAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTF ALAuxiliaryEffectSlotf;

LPALGENEFFECTS ALGenEffects;
LPALDELETEEFFECTS ALDeleteEffects;
LPALISEFFECT ALIsEffect;
LPALEFFECTI ALEffecti;
LPALEFFECTF ALEffectf;
LPALEFFECTFV ALEffectfv;

LPALGENFILTERS ALGenFilters;
LPALDELETEFILTERS ALDeleteFilters;
LPALISFILTER ALIsFilter;
LPALFILTERI ALFilteri;
LPALFILTERF ALFilterf;

HMODULE hALModule;

namespace AL
{
	VOID __fastcall LoadFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func)
	{
		StrCopy(buffer, prefix);
		StrCat(buffer, name);

		if (!*func)
			*func = GetProcAddress(hALModule, buffer);
	}

	BOOL __fastcall Load()
	{
		if (hALModule)
			return TRUE;

		if (!hALModule)
			hALModule = LoadLibrary("OPENAL32.dll");

		if (!hALModule)
			return FALSE;

		CHAR buffer[256];

#ifdef _DEBUG
		LoadFunction(buffer, PREFIX_AL, "GetError", (PROC*)&ALGetError);
#endif

		LoadFunction(buffer, PREFIX_ALC, "OpenDevice", (PROC*)&ALCOpenDevice);
		LoadFunction(buffer, PREFIX_ALC, "CloseDevice", (PROC*)&ALCCloseDevice);
		LoadFunction(buffer, PREFIX_ALC, "CreateContext", (PROC*)&ALCCreateContext);
		LoadFunction(buffer, PREFIX_ALC, "DestroyContext", (PROC*)&ALCDestroyContext);
		LoadFunction(buffer, PREFIX_ALC, "MakeContextCurrent", (PROC*)&ALCMakeContextCurrent);

		LoadFunction(buffer, PREFIX_AL, "DistanceModel", (PROC*)&ALDistanceModel);
		LoadFunction(buffer, PREFIX_AL, "Listenerfv", (PROC*)&ALListenerfv);
		LoadFunction(buffer, PREFIX_AL, "Listenerf", (PROC*)&ALListenerf);

		LoadFunction(buffer, PREFIX_AL, "GenSources", (PROC*)&ALGenSources);
		LoadFunction(buffer, PREFIX_AL, "DeleteSources", (PROC*)&ALDeleteSources);
		LoadFunction(buffer, PREFIX_AL, "GenBuffers", (PROC*)&ALGenBuffers);
		LoadFunction(buffer, PREFIX_AL, "DeleteBuffers", (PROC*)&ALDeleteBuffers);

		LoadFunction(buffer, PREFIX_AL, "BufferData", (PROC*)&ALBufferData);
		LoadFunction(buffer, PREFIX_AL, "SourceQueueBuffers", (PROC*)&ALSourceQueueBuffers);
		LoadFunction(buffer, PREFIX_AL, "SourceUnqueueBuffers", (PROC*)&ALSourceUnqueueBuffers);

		LoadFunction(buffer, PREFIX_AL, "GetSourcei", (PROC*)&ALGetSourcei);
		LoadFunction(buffer, PREFIX_AL, "Sourcei", (PROC*)&ALSourcei);
		LoadFunction(buffer, PREFIX_AL, "Sourcef", (PROC*)&ALSourcef);
		LoadFunction(buffer, PREFIX_AL, "Sourcefv", (PROC*)&ALSourcefv);
		LoadFunction(buffer, PREFIX_AL, "Source3i", (PROC*)&ALSource3i);

		LoadFunction(buffer, PREFIX_AL, "SourcePlay", (PROC*)&ALSourcePlay);
		LoadFunction(buffer, PREFIX_AL, "SourceStop", (PROC*)&ALSourceStop);
		LoadFunction(buffer, PREFIX_AL, "SourcePause", (PROC*)&ALSourcePause);

		LoadFunction(buffer, PREFIX_ALC, "IsExtensionPresent", (PROC*)&ALCIsExtensionPresent);
		LoadFunction(buffer, PREFIX_AL, "GetProcAddress", (PROC*)&ALGetProcAddress);

		return TRUE;
	}

	BOOL __fastcall ContextLoad()
	{
		if (!ALGetProcAddress)
			return FALSE;

		ALGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)ALGetProcAddress("alGenAuxiliaryEffectSlots");
		ALDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)ALGetProcAddress("alDeleteAuxiliaryEffectSlots");
		ALAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)ALGetProcAddress("alAuxiliaryEffectSloti");
		ALAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)ALGetProcAddress("alAuxiliaryEffectSlotf");

		ALGenEffects = (LPALGENEFFECTS)ALGetProcAddress("alGenEffects");
		ALDeleteEffects = (LPALDELETEEFFECTS)ALGetProcAddress("alDeleteEffects");
		ALIsEffect = (LPALISEFFECT)ALGetProcAddress("alIsEffect");
		ALEffecti = (LPALEFFECTI)ALGetProcAddress("alEffecti");
		ALEffectf = (LPALEFFECTF)ALGetProcAddress("alEffectf");
		ALEffectfv = (LPALEFFECTFV)ALGetProcAddress("alEffectfv");

		ALGenFilters = (LPALGENFILTERS)ALGetProcAddress("alGenFilters");
		ALDeleteFilters = (LPALDELETEFILTERS)ALGetProcAddress("alDeleteFilters");
		ALIsFilter = (LPALISFILTER)ALGetProcAddress("alIsFilter");
		ALFilteri = (LPALFILTERI)ALGetProcAddress("alFilteri");
		ALFilterf = (LPALFILTERF)ALGetProcAddress("alFilterf");

		return TRUE;
	}

	VOID __fastcall Free()
	{
		if (FreeLibrary(hALModule))
			hALModule = NULL;
	}

	VOID __fastcall UpdateEAXReverb(ALuint effect, const EFXEAXREVERBPROPERTIES* reverb)
	{
		ALEffectf(effect, AL_EAXREVERB_DENSITY, reverb->flDensity);
		ALEffectf(effect, AL_EAXREVERB_DIFFUSION, reverb->flDiffusion);
		ALEffectf(effect, AL_EAXREVERB_GAIN, reverb->flGain);
		ALEffectf(effect, AL_EAXREVERB_GAINHF, reverb->flGainHF);
		ALEffectf(effect, AL_EAXREVERB_GAINLF, reverb->flGainLF);
		ALEffectf(effect, AL_EAXREVERB_DECAY_TIME, reverb->flDecayTime);
		ALEffectf(effect, AL_EAXREVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
		ALEffectf(effect, AL_EAXREVERB_DECAY_LFRATIO, reverb->flDecayLFRatio);
		ALEffectf(effect, AL_EAXREVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
		ALEffectf(effect, AL_EAXREVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
		ALEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN, (ALfloat*)reverb->flReflectionsPan);
		ALEffectf(effect, AL_EAXREVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
		ALEffectf(effect, AL_EAXREVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
		ALEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN, (ALfloat*)reverb->flLateReverbPan);
		ALEffectf(effect, AL_EAXREVERB_ECHO_TIME, reverb->flEchoTime);
		ALEffectf(effect, AL_EAXREVERB_ECHO_DEPTH, reverb->flEchoDepth);
		ALEffectf(effect, AL_EAXREVERB_MODULATION_TIME, reverb->flModulationTime);
		ALEffectf(effect, AL_EAXREVERB_MODULATION_DEPTH, reverb->flModulationDepth);
		ALEffectf(effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
		ALEffectf(effect, AL_EAXREVERB_HFREFERENCE, reverb->flHFReference);
		ALEffectf(effect, AL_EAXREVERB_LFREFERENCE, reverb->flLFReference);
		ALEffectf(effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
		ALEffecti(effect, AL_EAXREVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
	}
}