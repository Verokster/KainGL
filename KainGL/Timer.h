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

#pragma once
#include "windows.h"

#define TIMEOUT_PC 10
#define TIMEOUT_PS 25

typedef VOID(APIENTRY *PTIMERAPCROUTINE)(
	LPVOID lpArgToCompletionRoutine,
	DWORD dwTimerLowValue,
	DWORD dwTimerHighValue
	);

typedef HANDLE(WINAPI *CREATEWAITABLETIMER)(
	LPSECURITY_ATTRIBUTES lpTimerAttributes,
	BOOL bManualReset,
	LPCSTR lpTimerName
	);

typedef BOOL(WINAPI *SETWAITABLETIMER)(
	HANDLE hTimer,
	const LARGE_INTEGER *lpDueTime,
	LONG lPeriod,
	PTIMERAPCROUTINE pfnCompletionRoutine,
	LPVOID lpArgToCompletionRoutine,
	BOOL fResume
	);

typedef BOOL(WINAPI *CANCELWAITABLETIMER)(
	HANDLE hTimer
	);

namespace Timer
{
	VOID __fastcall Load();
	VOID __fastcall Unload();
	VOID __fastcall Start(LONG timeout);
	VOID __fastcall Stop();
	VOID __fastcall Wait();
}