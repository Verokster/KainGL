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

#include "windows.h"
#include "al.h"
#include "alc.h"
#include "efx.h"
#include "efx-creative.h"
#include "EFX-Util.h"

#define PREFIX_AL "al"
#define PREFIX_ALC "alc"

#ifdef _DEBUG
typedef ALenum(__cdecl *ALGETERROR)();
extern ALGETERROR ALGetError;
#endif

typedef ALCdevice*(__cdecl *ALCOPENDEVICE)(const ALCchar* devicename);
typedef ALCboolean(__cdecl *ALCCLOSEDEVICE)(ALCdevice* device);
typedef ALCcontext*(__cdecl *ALCCREATECONTEXT)(ALCdevice* device, const ALCint* attrlist);
typedef VOID(__cdecl *ALCDESTROYCONTEXT)(ALCcontext* context);
typedef ALCboolean(__cdecl *ALCMAKECONTEXTCURRENT)(ALCcontext *context);

typedef VOID(__cdecl *ALDISTANCEMODEL)(ALenum distanceModel);
typedef VOID(__cdecl *ALLISTENERFV)(ALenum param, const ALfloat* values);
typedef VOID(__cdecl *ALLISTENERF)(ALenum param, ALfloat value);

typedef VOID(__cdecl *ALGENSOURCES)(ALsizei n, ALuint* sources);
typedef VOID(__cdecl *ALDELETESOURCES)(ALsizei n, const ALuint* sources);
typedef VOID(__cdecl *ALGENBUFFERS)(ALsizei n, ALuint* buffers);
typedef VOID(__cdecl *ALDELETEBUFFERS)(ALsizei n, const ALuint* buffers);

typedef VOID(__cdecl *ALBUFFERDATA)(ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);
typedef VOID(__cdecl *ALSOURCEQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, const ALuint* bids);
typedef VOID(__cdecl *ALSOURCEUNQUEUEBUFFERS)(ALuint sid, ALsizei numEntries, ALuint *bids);

typedef VOID(__cdecl *ALGETSOURCEI)(ALuint sid, ALenum param, ALint* value);
typedef VOID(__cdecl *ALSOURCEI)(ALuint sid, ALenum param, ALint value);
typedef VOID(__cdecl *ALSOURCEF)(ALuint sid, ALenum param, ALfloat value);
typedef VOID(__cdecl *ALSOURCEFV)(ALuint sid, ALenum param, const ALfloat* values);
typedef VOID(__cdecl *ALSOURCE3I)(ALuint sid, ALenum param, ALint value1, ALint value2, ALint value3);

typedef VOID(__cdecl *ALSOURCEPLAY)(ALuint sid);
typedef VOID(__cdecl *ALSOURCESTOP)(ALuint sid);
typedef VOID(__cdecl *ALSOURCEPAUSE)(ALuint sid);

typedef ALCboolean(__cdecl *ALCISEXTENSIONPRESENT)(ALCdevice *device, const ALCchar *extname);
typedef VOID*(__cdecl *ALGETPROCADDRESS)(const ALchar* fname);
typedef VOID*(__cdecl *ALCGETPROCADDRESS)(ALCdevice* device, const ALCchar* funcname);

extern ALCOPENDEVICE ALCOpenDevice;
extern ALCCLOSEDEVICE ALCCloseDevice;
extern ALCCREATECONTEXT ALCCreateContext;
extern ALCDESTROYCONTEXT ALCDestroyContext;
extern ALCMAKECONTEXTCURRENT ALCMakeContextCurrent;

extern ALDISTANCEMODEL ALDistanceModel;
extern ALLISTENERFV ALListenerfv;
extern ALLISTENERF ALListenerf;

extern ALGENSOURCES ALGenSources;
extern ALDELETESOURCES ALDeleteSources;
extern ALGENBUFFERS ALGenBuffers;
extern ALDELETEBUFFERS ALDeleteBuffers;

extern ALBUFFERDATA ALBufferData;
extern ALSOURCEQUEUEBUFFERS ALSourceQueueBuffers;
extern ALSOURCEUNQUEUEBUFFERS ALSourceUnqueueBuffers;

extern ALGETSOURCEI ALGetSourcei;
extern ALSOURCEI ALSourcei;
extern ALSOURCEF ALSourcef;
extern ALSOURCEFV ALSourcefv;
extern ALSOURCE3I ALSource3i;

extern ALSOURCEPLAY ALSourcePlay;
extern ALSOURCESTOP ALSourceStop;
extern ALSOURCEPAUSE ALSourcePause;

extern ALCISEXTENSIONPRESENT ALCIsExtensionPresent;
extern ALGETPROCADDRESS ALGetProcAddress;

extern LPALGENAUXILIARYEFFECTSLOTS ALGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS ALDeleteAuxiliaryEffectSlots;
extern LPALAUXILIARYEFFECTSLOTI ALAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTF ALAuxiliaryEffectSlotf;

extern LPALGENEFFECTS ALGenEffects;
extern LPALDELETEEFFECTS ALDeleteEffects;
extern LPALISEFFECT ALIsEffect;
extern LPALEFFECTI ALEffecti;
extern LPALEFFECTF ALEffectf;
extern LPALEFFECTFV ALEffectfV;

extern LPALGENFILTERS ALGenFilters;
extern LPALDELETEFILTERS ALDeleteFilters;
extern LPALISFILTER ALIsFilter;
extern LPALFILTERI ALFilteri;
extern LPALFILTERF ALFilterf;

extern const EFXEAXREVERBPROPERTIES EFX_REVERB_CASTLE_SHORTPASSAGE;
//extern const EFXEAXREVERBPROPERTIES EFX_REVERB_OUTDOORS_VALLEY;
extern const EFXEAXREVERBPROPERTIES EFX_REVERB_OUTDOORS_DEEPCANYON;

namespace AL
{
	BOOL __fastcall Load();
	BOOL __fastcall ContextLoad();
	VOID __fastcall Free();
	VOID __fastcall UpdateEAXReverb(ALuint effect, const EFXEAXREVERBPROPERTIES* reverb);
}