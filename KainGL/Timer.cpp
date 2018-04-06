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
#include "Timer.h"

CREATEWAITABLETIMER UserCreateWaitableTimer;
SETWAITABLETIMER UserSetWaitableTimer;
CANCELWAITABLETIMER UserCancelWaitableTimer;

HMODULE hKernel32;
HANDLE hTimer;
BOOL isStarted;
BOOL isSupported;

namespace Timer
{
	VOID CALLBACK Notify(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue) { };

	VOID __fastcall Load()
	{
		if (!hKernel32)
		{
			hKernel32 = GetModuleHandle("KERNEL32.DLL");
			if (hKernel32)
			{
				UserCreateWaitableTimer = (CREATEWAITABLETIMER)GetProcAddress(hKernel32, "CreateWaitableTimerA");
				UserSetWaitableTimer = (SETWAITABLETIMER)GetProcAddress(hKernel32, "SetWaitableTimer");
				UserCancelWaitableTimer = (CANCELWAITABLETIMER)GetProcAddress(hKernel32, "CancelWaitableTimer");

				isSupported = UserCreateWaitableTimer && UserSetWaitableTimer && UserCancelWaitableTimer;
			}
		}

		if (!hTimer && isSupported)
			hTimer = UserCreateWaitableTimer(NULL, FALSE, NULL);
	}

	VOID __fastcall Unload()
	{
		if (hTimer)
		{
			CloseHandle(hTimer);
			hTimer = NULL;
		}
	}

	VOID __fastcall Start(LONG timeout)
	{
		if (!isSupported)
			return;

		Stop();

		isStarted = TRUE;
		LARGE_INTEGER liDueTime = { NULL };
		UserSetWaitableTimer(hTimer, &liDueTime, timeout, Notify, NULL, FALSE);
	}

	VOID __fastcall Stop()
	{
		if (isStarted && isSupported)
		{
			isStarted = FALSE;
			UserCancelWaitableTimer(hTimer);
		}
	}

	VOID __fastcall Wait()
	{
		if (isSupported)
			SleepEx(INFINITE, TRUE);
	}
}