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
#include "Hooks.h"

struct Mode
{
	BOOL hiRes;
	BOOL hiColor;
	BOOL window;
	BOOL interlaced;
	BOOL upscale;
} modes[5] = {
	0, 1, 0, 1, 0,	// 240i
	0, 1, 0, 0, 0,	// 240p
	0, 1, 1, 0, 0,	// Windowed
	1, 1, 0, 1, 0,	// 480i
	1, 1, 0, 0, 0	// 480p
};

ModePtr flags;

DWORD GetFlag()
{
	for (DWORD i = 0; i < sizeof(modes) / sizeof(Mode); ++i)
	{
		Mode* mode = &modes[i];
		if (mode->hiRes == *flags.hiRes &&
			mode->hiColor == *flags.hiColor &&
			mode->window == *flags.window &&
			mode->interlaced == *flags.interlaced &&
			mode->upscale == *flags.upscale)
			return i;
	}

	return 0;
}

DWORD back_0042789C;
VOID __declspec(naked) hook_0042785B()
{
	__asm
	{
		call GetFlag
		jmp back_0042789C
	}
}

namespace Hooks
{
	VOID Patch_Modes(HOOKER hooker)
	{
		DWORD baseOffset = GetBaseOffset(hooker);

		flags = { (BOOL*)f(0x008E81E0), (BOOL*)f(0x004C0EC0), (BOOL*)f(0x008E81D8), (BOOL*)f(0x008E81D0), (BOOL*)f(0x008E81DC) };

		MemoryCopy((VOID*)(0x004BCF90 + baseOffset), modes, sizeof(modes));
		PatchHook(hooker, 0x0042785B, hook_0042785B);
		back_0042789C = f(0x0042789C);

		//PatchByte(0x0044E6B1 + 1, 0x1B);
		PatchJump(hooker, 0x00429AFF, 0x00429B24);
		PatchByte(hooker, 0x00429BBD + 2, 3);

		// Remove interlace from logic
		PatchByte(hooker, 0x0045E855, 0x31);
		PatchByte(hooker, 0x0045E4D4, 0xEB);
		PatchNop(hooker, 0x00442114, 2); // at GetCursorPosition

		PatchNop(hooker, 0x00441EA0, 2);
		PatchNop(hooker, 0x00441F5D, 2);
		PatchNop(hooker, 0x00441FDA, 2);
		PatchNop(hooker, 0x0044206A, 2);

		PatchByte(hooker, 0x00429747, 0xEB);

		PatchNop(hooker, 0x00429564, 2);

		PatchNop(hooker, 0x00429106, 2);
		PatchByte(hooker, 0x00429260, 0xEB);

		PatchNop(hooker, 0x00428624, 2);
		PatchByte(hooker, 0x004289A6, 0xEB);

		PatchNop(hooker, 0x004283F4, 2);

		// Correct menu and map for 320
		PatchNop(hooker, 0x004284F1, 6);
		PatchNop(hooker, 0x00428501, 6);
		PatchNop(hooker, 0x00428511, 6);
		PatchNop(hooker, 0x00428521, 6);
		PatchNop(hooker, 0x0042852C, 6);
	}
}