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
#include "Windowsx.h"
#include "Hooks.h"
#include "Config.h"
#include "Main.h"

BOOL __stdcall GetCursorPosHook(LPPOINT lpPoint)
{
	lpPoint->x = GET_X_LPARAM(mousePos);
	lpPoint->y = GET_Y_LPARAM(mousePos);
	return TRUE;
};

BOOL __stdcall SetCursorPosHook(INT X, INT Y) { return TRUE; };

namespace Hooks
{
	VOID Patch_Mouse()
	{
		PatchFunction("GetCursorPos", GetCursorPosHook);
		PatchFunction("SetCursorPos", SetCursorPosHook);

		PatchByte(0x00444B68, 0xC3); // Prevent map move by cursor
		PatchByte(0x00428A1D + 1, 1); // add no wait flag 
	}
}