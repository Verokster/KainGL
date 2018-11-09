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
#include "Hooks.h"
#include "Mmsystem.h"
#include "Config.h"

#define M_PI       3.14159265358979323846

#define STICK_CENTER 1
#define STICK_LEFT 0
#define STICK_RIGHT 2
#define STICK_TOP 0
#define STICK_BOTTOM 2

BOOL* xJoyListConnected = (BOOL*)0x0058CCF8;
BOOL xJoyListCheck[MAX_CONTROLLERS];

DWORD __stdcall joyGetNumDevsHook() { return MAX_CONTROLLERS; }

MMRESULT __stdcall joyGetPosHook(UINT uJoyID, LPJOYINFO pji)
{
	if (uJoyID >= MAX_CONTROLLERS)
		return MMSYSERR_NODRIVER;

	XINPUT_STATE state;
	DWORD dwResult = InputGetState(uJoyID, &state);
	if (dwResult == ERROR_SUCCESS)
	{
		xJoyListCheck[uJoyID] = TRUE;

		MemoryZero(pji, sizeof(JOYINFO));

		if (state.Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN))
		{
			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
				pji->wXpos = STICK_RIGHT;
			else if (!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
				pji->wXpos = STICK_CENTER;

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
				pji->wYpos = STICK_RIGHT;
			else if (!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP))
				pji->wYpos = STICK_CENTER;
		}
		else
		{
			DWORD g = (DWORD)MathSqrt(DOUBLE(state.Gamepad.sThumbLX * state.Gamepad.sThumbLX + state.Gamepad.sThumbLY * state.Gamepad.sThumbLY));
			if (g >= 22938) // 70%
			{
				INT agl = (INT)MathFloor(MathAtan2((DOUBLE)state.Gamepad.sThumbLY, (DOUBLE)state.Gamepad.sThumbLX) * 8.0f / M_PI) + 8;
				switch (agl)
				{
				case 1:
				case 2:
					pji->wXpos = STICK_LEFT;
					pji->wYpos = STICK_BOTTOM;
					break;

				case 3:
				case 4:
					pji->wXpos = STICK_CENTER;
					pji->wYpos = STICK_BOTTOM;
					break;

				case 5:
				case 6:
					pji->wXpos = STICK_RIGHT;
					pji->wYpos = STICK_BOTTOM;
					break;

				case 7:
				case 8:
					pji->wXpos = STICK_RIGHT;
					pji->wYpos = STICK_CENTER;
					break;

				case 9:
				case 10:
					pji->wXpos = STICK_RIGHT;
					pji->wYpos = STICK_TOP;
					break;

				case 11:
				case 12:
					pji->wXpos = STICK_CENTER;
					pji->wYpos = STICK_TOP;
					break;

				case 13:
				case 14:
					pji->wXpos = STICK_LEFT;
					pji->wYpos = STICK_TOP;
					break;

				default:
					pji->wXpos = STICK_LEFT;
					pji->wYpos = STICK_CENTER;
					break;
				}
			}
			else
				pji->wYpos = pji->wXpos = STICK_CENTER;
		}

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
			pji->wButtons |= 0x1;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			pji->wButtons |= 0x2;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
			pji->wButtons |= 0x4;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
			pji->wButtons |= 0x8;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
			pji->wButtons |= 0x10;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			pji->wButtons |= 0x20;

		if (state.Gamepad.bRightTrigger >= 64)
			pji->wButtons |= 0x40;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			pji->wButtons |= 0x80;

		if (state.Gamepad.bLeftTrigger >= 64)
			pji->wButtons |= 0x100;

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
			pji->wButtons |= 0x200;
	}
	else
		xJoyListCheck[uJoyID] = FALSE;

	return dwResult;
}

MMRESULT __stdcall joyGetDevCapsAHook(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
	if (uJoyID >= MAX_CONTROLLERS)
		return MMSYSERR_NODRIVER;

	XINPUT_CAPABILITIES caps;
	DWORD dwResult = InputGetCapabilities(uJoyID, XINPUT_FLAG_GAMEPAD, &caps);
	if (dwResult == ERROR_SUCCESS)
	{
		MemoryZero(pjc, cbjc);
		pjc->wNumButtons = 32;
		pjc->wXmax = pjc->wYmax = 2;
	}

	return dwResult;
}

DWORD __stdcall GetJoyID()
{
	for (DWORD i = 0; i < MAX_CONTROLLERS; ++i)
	{
		if (xJoyListConnected[i] && xJoyListCheck[i])
			return i;
	}

	return 0;
}

DWORD back_00427552 = 0x00427552;
VOID __declspec(naked) hook_00427536()
{
	__asm
	{
		CALL GetJoyID
		MOV EBX, EAX

		JMP back_00427552
	}
}

BYTE indices[10] = { 3, 1, 0, 5, 2, 7, 9, 6, 8, 4 };
BOOL isXbox;
VOID __stdcall ChangeIconIndex(DWORD index, DWORD* position)
{
	POINT pos;
	if (index < 10)
	{
		index = indices[index];
		pos.x = (isXbox ? 16 : 271) + 320;
		pos.y = 12 * index + 13 + 256;
	}
	else
	{
		pos.x = 17 * (index % 16) + 16 + 320;
		pos.y = 12 * (index / 16) + 171 + 256;
	}

	*position = pos.x | (pos.y << 16);
}

DWORD back_00414478 = 0x00414478;
VOID __declspec(naked) hook_0041443B()
{
	__asm
	{
		PUSH ESP
		PUSH EBX
		CALL ChangeIconIndex
		JMP back_00414478
	}
}

// Controls setup
//BYTE* controlsState = (BYTE*)0x00511AFE;
INT controState;
BOOL isController;

DWORD* joyList = (DWORD*)0x004BCCEC;
DWORD* joyPsList = (DWORD*)0x004BCD6C;
DWORD* joyXboxList = (DWORD*)0x004BCDEC;
DWORD* joyTempList = (DWORD*)0x00511A74;

BYTE* keyList = (BYTE*)0x004BD1FB;
BYTE* keyDefaultList = (BYTE*)0x004BD1F1;
BYTE* keyTempList = (BYTE*)0x00511AF4;

VOID __stdcall InitControllesSetup()
{
	controState = 0;
	isXbox = configOtherXboxConfig;
}

DWORD back_00413A18 = 0x00413A18;
VOID __declspec(naked) hook_00413A12()
{
	__asm
	{
		CALL InitControllesSetup
		JMP back_00413A18
	}
}

DWORD sub_004467FC = 0x004467FC;
VOID __stdcall PressToogle(BOOL isRight)
{
	if (isRight)
	{
		if (controState == 1)
			return;
		++controState;
	}
	else
	{
		if (controState == (isController ? -1 : 0))
			return;
		--controState;
	}

	((VOID(*)())sub_004467FC)(); // Heartbeat sound

	if (!controState)
	{
		MemoryCopy(joyList, joyTempList, 32 * sizeof(DWORD));
		MemoryCopy(keyList, keyTempList, 10 * sizeof(BYTE));
		isXbox = configOtherXboxConfig;
	}
	else
	{
		MemoryCopy(joyTempList, joyList, 32 * sizeof(DWORD));
		MemoryCopy(keyTempList, keyList, 10 * sizeof(BYTE));

		isXbox = controState == 1;

		DWORD* joyDefaultList = isXbox ? joyXboxList : joyPsList;
		MemoryCopy(joyList, joyDefaultList, 32 * sizeof(DWORD));
		MemoryCopy(keyList, keyDefaultList, 10 * sizeof(BYTE));
	}
}

DWORD back_00413DCD = 0x00413DCD;
// left
VOID __declspec(naked) hook_00413BBA()
{
	__asm
	{
		PUSH 0
		CALL PressToogle
		JMP back_00413DCD
	}
}

// right
VOID __declspec(naked) hook_00413BF5()
{
	__asm
	{
		PUSH 1
		CALL PressToogle
		JMP back_00413DCD
	}
}

VOID __stdcall AcceptControls()
{
	controState = 0;
	configOtherXboxConfig = isXbox;
}

DWORD back_00413D08 = 0x00413D08;
VOID __declspec(naked) hook_00413CF7()
{
	__asm
	{
		CALL AcceptControls
		JMP back_00413D08
	}
}

DWORD back_00413D87 = 0x00413D87;
VOID __declspec(naked) hook_00413D76()
{
	__asm
	{
		CALL AcceptControls
		JMP back_00413D87
	}
}

VOID __stdcall SetControls()
{
	AcceptControls();
	Config::Set(CONFIG_OTHER, CONFIG_OTHER_XBOX_CONFIG, configOtherXboxConfig);
}

VOID __declspec(naked) hook_00413F0B()
{
	__asm
	{
		POP EBP
		POP EDI
		POP EBX
		JMP SetControls
	}
}

namespace Hooks
{
	VOID Patch_Input()
	{
		if (InputGetState)
		{
			XINPUT_STATE state;
			for (DWORD i = 0; i < MAX_CONTROLLERS; ++i)
			{
				if (InputGetState(i, &state) == ERROR_SUCCESS)
				{
					isController = TRUE;

					// Chnage icon position
					PatchHook(0x0041443B, hook_0041443B);
					back_00414478 += baseAddress;

					PatchFunction("joyGetNumDevs", joyGetNumDevsHook);
					PatchFunction("joyGetPos", joyGetPosHook);
					PatchFunction("joyGetDevCapsA", joyGetDevCapsAHook);
					PatchByte(0x0042747F + 2, MAX_CONTROLLERS);
					PatchHook(0x00427536, hook_00427536);
					back_00427552 += baseAddress;

					// Accept joy controls
					PatchHook(0x00413F0B, hook_00413F0B);

					break;
				}
			}
		}

		// Correct joy buttons draw position
		PatchNop(0x0041446C, 2);
		PatchDWord(0x00414213 + 1, 17);

		// Add PS / XBOX switch
		PatchHook(0x00413A12, hook_00413A12);
		back_00413A18 += baseAddress;

		sub_004467FC += baseAddress;

		PatchHook(0x00413BBA, hook_00413BBA);
		PatchHook(0x00413BF5, hook_00413BF5);
		back_00413DCD += baseAddress;

		PatchHook(0x00413CF7, hook_00413CF7);
		back_00413D08 += baseAddress;
		PatchHook(0x00413D76, hook_00413D76);
		back_00413D87 += baseAddress;
	}
}