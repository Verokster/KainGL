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

#pragma once
#include "windows.h"
#include "OpenDraw.h"
#include "OpenSound.h"

extern OpenDraw* ddrawList;
extern IOpenDirectSound* dsoundList;

namespace Main
{
	VOID __stdcall SetSyncDraw();

	HRESULT __stdcall DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter);

	HRESULT __stdcall DirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext);

	HRESULT __stdcall DirectSoundCreate(GUID* lpGUID, LPDIRECTSOUND *lplpDS, LPUNKNOWN pUnkOuter);

	VOID __fastcall ShowError(CHAR* message, CHAR* file, DWORD line);

#ifdef _DEBUG
	VOID __fastcall CheckGLError(CHAR* file, DWORD line);
	VOID __fastcall CheckAlError(CHAR* file, DWORD line);
#endif
}