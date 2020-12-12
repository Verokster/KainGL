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
#include "Main.h"

namespace Hooks
{
#pragma optimize("s", on)
	VOID Patch_Library(HOOKER hooker)
	{
		PatchImportByName(hooker, "DirectDrawEnumerateA", Main::DirectDrawEnumerateA);
		PatchImportByName(hooker, "DirectDrawCreate", Main::DirectDrawCreate);
		PatchImportByName(hooker, "DirectSoundCreate", Main::DirectSoundCreate, (DWORD*)&DSCreate);

		PatchHook(hooker, 0x0046F680, memset);
		PatchHook(hooker, 0x0046F64D, memcpy);
		PatchHook(hooker, 0x0046FBD6, memcmp);
	}
#pragma optimize("", on)
}