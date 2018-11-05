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
#include "TextRenderer.h"

TextRenderer::TextRenderer(HDC hDc, DWORD width, DWORD height)
{
	this->width = width;
	this->height = height;

	BITMAPV5HEADER bmiHeader;
	MemoryZero(&bmiHeader, sizeof(BITMAPV5HEADER));
	bmiHeader.bV5Size = sizeof(BITMAPV5HEADER);
	bmiHeader.bV5Width = width;
	bmiHeader.bV5Height = -*(LONG*)&height;
	bmiHeader.bV5Planes = 1;
	bmiHeader.bV5BitCount = 32;
	bmiHeader.bV5Compression = BI_BITFIELDS;//BI_RGB;
	bmiHeader.bV5XPelsPerMeter = 1;
	bmiHeader.bV5YPelsPerMeter = 1;
	bmiHeader.bV5RedMask = 0x000000FF;
	bmiHeader.bV5GreenMask = 0x0000FF00;
	bmiHeader.bV5BlueMask = 0x00FF0000;
	bmiHeader.bV5AlphaMask = 0xFF000000;

	this->hDc = CreateCompatibleDC(hDc);
	this->hBmp = CreateDIBSection(this->hDc, (BITMAPINFO*)&bmiHeader, DIB_RGB_COLORS, &this->dibData, NULL, 0);
	SelectObject(this->hDc, hBmp);
}

TextRenderer::~TextRenderer()
{
	DeleteObject(this->hBmp);
	DeleteDC(this->hDc);
}

VOID TextRenderer::Draw(DRAWTEXT TextDraw, VOID* text, HFONT hFont, COLORREF color, RECT* rect, DWORD flags)
{
	SetBkMode(this->hDc, 1);
	SelectObject(this->hDc, hFont);

	DWORD dir = flags & TR_RIGHT ? DT_RIGHT : flags & TR_CENTER ? DT_CENTER : DT_LEFT;

	POINT offset;
	if ((flags == (TR_LEFT | TR_TOP)) && (flags != TR_CALCULATE))
	{
		offset.x = 0;
		offset.y = 0;
	}
	else
	{
		RECT old = *rect;
		TextDraw(this->hDc, text, -1, rect, dir | DT_WORDBREAK | DT_CALCRECT);
		offset.x = flags & TR_LEFT ? 0 : flags & TR_RIGHT ? old.right - rect->right : (((old.right - old.left) - (rect->right - rect->left)) >> 1);
		offset.y = flags & TR_TOP ? 0 : flags & TR_BOTTOM ? old.bottom - rect->bottom : (((old.bottom - old.top) - (rect->bottom - rect->top)) >> 1);
	}

	if (flags & TR_SHADOW)
	{
		OffsetRect(rect, offset.x + 1, offset.y + 1);
		SetTextColor(this->hDc, RGB(0, 0, 0));
		TextDraw(this->hDc, text, -1, rect, dir | DT_WORDBREAK);
		OffsetRect(rect, -1, -1);
	}
	else
		OffsetRect(rect, offset.x, offset.y);

	SetTextColor(this->hDc, color);
	TextDraw(this->hDc, text, -1, rect, dir | DT_WORDBREAK);
}

VOID TextRenderer::DrawA(CHAR* text, HFONT hFont, COLORREF color, RECT* rect, DWORD flags)
{
	this->Draw((DRAWTEXT)DrawTextA, text, hFont, color, rect, flags);
}

VOID TextRenderer::DrawW(WCHAR* text, HFONT hFont, COLORREF color, RECT* rect, DWORD flags)
{
	this->Draw((DRAWTEXT)DrawTextW, text, hFont, color, rect, flags);
}