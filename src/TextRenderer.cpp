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
#include "TextRenderer.h"
#include "Main.h"

TextRenderer::TextRenderer(HDC hDc, DWORD width, DWORD height)
{
	this->width = width;
	this->height = height;

	this->hDc = CreateCompatibleDC(hDc);
	if (!this->hDc)
		Main::ShowError("Cannot create DIB section", __FILE__, __LINE__);

	BITMAPV4HEADER bmiHeader;
	MemoryZero(&bmiHeader, sizeof(BITMAPV4HEADER));
	bmiHeader.bV4Size = sizeof(BITMAPV4HEADER);
	bmiHeader.bV4Width = width;
	bmiHeader.bV4Height = -*(LONG*)&height;
	bmiHeader.bV4Planes = 1;
	bmiHeader.bV4BitCount = 32;
	bmiHeader.bV4V4Compression = BI_BITFIELDS;
	bmiHeader.bV4RedMask = 0x000000FF;
	bmiHeader.bV4GreenMask = 0x0000FF00;
	bmiHeader.bV4BlueMask = 0x00FF0000;
	bmiHeader.bV4AlphaMask = 0xFF000000;

	this->swapColor = FALSE;
	this->hBmp = CreateDIBSection(this->hDc, (BITMAPINFO*)&bmiHeader, DIB_RGB_COLORS, &this->dibData, NULL, 0);
	if (!this->hBmp)
	{
		bmiHeader.bV4V4Compression = BI_RGB;
		bmiHeader.bV4RedMask = 0;
		bmiHeader.bV4GreenMask = 0;
		bmiHeader.bV4BlueMask = 0;
		bmiHeader.bV4AlphaMask = 0;

		this->swapColor = TRUE;
		this->hBmp = CreateDIBSection(this->hDc, (BITMAPINFO*)&bmiHeader, DIB_RGB_COLORS, &this->dibData, NULL, 0);
	}

	if (!this->hBmp)
		Main::ShowError("Cannot create DIB section", __FILE__, __LINE__);

	SelectObject(this->hDc, hBmp);
}

TextRenderer::~TextRenderer()
{
	DeleteObject(this->hBmp);
	DeleteDC(this->hDc);
}

VOID TextRenderer::Draw(DRAWTEXT TextDraw, VOID* text, HFONT hFont, COLORREF color, COLORREF background, RECT* rect, RECT* padding, DWORD flags)
{
	SetBkMode(this->hDc, 1);
	SelectObject(this->hDc, hFont);

	if (padding)
	{
		rect->left += padding->left;
		rect->top += padding->top;
		rect->right -= padding->right;
		rect->bottom -= padding->bottom;
	}

	RECT old = *rect;
	DWORD dir = flags & TR_RIGHT ? DT_RIGHT : flags & TR_CENTER ? DT_CENTER : DT_LEFT;
	TextDraw(this->hDc, text, -1, rect, dir | DT_WORDBREAK | DT_CALCRECT);

	POINT offset = {
		flags & TR_LEFT ? 0 : flags & TR_RIGHT ? old.right - rect->right : (((old.right - old.left) - (rect->right - rect->left)) >> 1),
		flags & TR_TOP ? 0 : flags & TR_BOTTOM ? old.bottom - rect->bottom : (((old.bottom - old.top) - (rect->bottom - rect->top)) >> 1)
	};

	if (flags & TR_BACKGROUND)
	{
		RECT plane = *rect;
		OffsetRect(&plane, offset.x, offset.y);

		if (padding)
		{
			plane.left -= padding->left;
			plane.top -= padding->top;
			plane.right += padding->right;
			plane.bottom += padding->bottom;
		}

		if (plane.left < 0)
			plane.left = 0;

		if (plane.top < 0)
			plane.top = 0;

		if (plane.right > *(LONG*)&this->width)
			plane.right = *(LONG*)&this->width;

		if (plane.bottom > *(LONG*)&this->height)
			plane.bottom = *(LONG*)&this->height;

		DWORD br = (*(DWORD*)&background & 0x000000FF) * ALPHA_BIAS;
		DWORD bg = (*(DWORD*)&background & 0x0000FF00) * ALPHA_BIAS;
		DWORD bb = (*(DWORD*)&background & 0x00FF0000) * ALPHA_BIAS;
		DWORD bias = 0xFF - ALPHA_BIAS;

		DWORD* pixData = (DWORD*)this->dibData + this->width * plane.top + plane.left;

		DWORD cx = plane.right - plane.left;
		DWORD height = plane.bottom - plane.top;
		do
		{
			DWORD* pix = pixData;

			DWORD width = cx;
			do
			{
				DWORD px = *pix;
				DWORD r = (((px & 0x000000FF) * bias + br) / 0xFF) & 0x000000FF;
				DWORD g = (((px & 0x0000FF00) * bias + bg) / 0xFF) & 0x0000FF00;
				DWORD b = (((px & 0x00FF0000) * bias + bb) / 0xFF) & 0x00FF0000;
				*pix++ = r | g | b;
			} while (--width);

			pixData += this->width;
		} while (--height);
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

	if (this->swapColor)
		*(DWORD*)&color = _byteswap_ulong(_rotl(*(DWORD*)&color, 8));
	 
	SetTextColor(this->hDc, color);
	TextDraw(this->hDc, text, -1, rect, dir | DT_WORDBREAK);
}

VOID TextRenderer::DrawA(CHAR* text, HFONT hFont, COLORREF color, DWORD backColor, RECT* rect, RECT* padding, DWORD flags)
{
	this->Draw((DRAWTEXT)DrawTextA, text, hFont, color, backColor, rect, padding, flags);
}

VOID TextRenderer::DrawW(WCHAR* text, HFONT hFont, COLORREF color, DWORD backColor, RECT* rect, RECT* padding, DWORD flags)
{
	this->Draw((DRAWTEXT)DrawTextUni, text, hFont, color, backColor, rect, padding, flags);
}