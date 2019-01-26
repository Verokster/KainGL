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
#include "Allocation.h"

#define TR_LEFT			0x0001
#define TR_CENTER		0x0002
#define TR_RIGHT		0x0004

#define TR_TOP			0x0008
#define TR_MIDDLE		0x0010
#define TR_BOTTOM		0x0020

#define TR_SHADOW		0x0040
#define TR_BACKGROUND	0x0080
#define TR_CALCULATE	0x0100

#define ALPHA_BIAS		0xA0

typedef INT(__stdcall *DRAWTEXT)(HDC, VOID*, INT, LPRECT, UINT);

class TextRenderer : public Allocation
{
private:
	HBITMAP hBmp;
	HDC hDc;
	DWORD width;
	DWORD height;
	BOOL swapColor;

	VOID Draw(DRAWTEXT TextDraw, VOID* text, HFONT hFont, COLORREF color, COLORREF backColor, RECT* rect, RECT* padding, DWORD flags);

public:
	VOID* dibData;

	TextRenderer(HDC hDc, DWORD width, DWORD height);
	~TextRenderer();

	VOID DrawA(CHAR* text, HFONT hFont, COLORREF color, COLORREF backColor, RECT* rect, RECT* padding, DWORD flags);
	VOID DrawW(WCHAR* text, HFONT hFont, COLORREF color, COLORREF backColor, RECT* rect, RECT* padding, DWORD flags);
};

