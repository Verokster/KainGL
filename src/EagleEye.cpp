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
#include "Hooks.h"

DWORD back_00423E9B = 0x00423E9B;
DWORD sub_0045DE6C = 0x0045DE6C;
VOID __declspec(naked) hook_00423E96()
{
	__asm
	{
		XOR ECX, ECX
		POP EAX

		PUSH 0x34
		PUSH ECX
		PUSH EAX
		CALL MemorySet

		POP EAX
		ADD ESP, 8
		PUSH EAX

		CALL sub_0045DE6C
		JMP back_00423E9B
	}
}

namespace Hooks
{
	VOID Patch_EagleEye()
	{
		PatchHook(0x00423E96, hook_00423E96); // Clear object after initialization
		back_00423E9B += baseOffset;
		sub_0045DE6C += baseOffset;
	}
}