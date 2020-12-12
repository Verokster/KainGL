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
#include "Config.h"

struct RGB {
	BYTE r;
	BYTE g;
	BYTE b;
};

struct RGBFloat {
	FLOAT r;
	FLOAT g;
	FLOAT b;
} buffer[480][640];

VOID __cdecl ConvertRGBA8888To5551(BYTE* data, INT width, INT height)
{
	if (width > 0 && height > 0)
	{
		BYTE checkDrawOmHiRes = *(BYTE*)0x0058F284;
		DWORD flagHires = *(DWORD*)0x008E81E0;

		if (checkDrawOmHiRes && flagHires)
		{
			width <<= 1;
			height <<= 1;
		}

		for (INT y = 0; y < height; ++y)
		{
			for (INT x = 0; x < width; ++x)
			{
				RGB* pix = (RGB*)data + y * width + x;
				RGBFloat* bf = &buffer[y][x];
				bf->r = (FLOAT)pix->r / 255.0f;
				bf->g = (FLOAT)pix->g / 255.0f;
				bf->b = (FLOAT)pix->b / 255.0f;
			}
		}

		for (INT y = 0; y < height; ++y)
		{
			for (INT x = 0; x < width; ++x)
			{
				RGBFloat* oldPix = &buffer[y][x];

				INT r = INT(MathRound(31.0f * oldPix->r));
				INT g = INT(MathRound(31.0f * oldPix->g));
				INT b = INT(MathRound(31.0f * oldPix->b));

				r = min(max(r, 0), 31);
				g = min(max(g, 0), 31);
				b = min(max(b, 0), 31);

				*((WORD*)data + y * width + x) = (r << 10) | (g << 5) | b;

				RGBFloat newPix;
				newPix.r = (FLOAT)r / 31.0f;
				newPix.g = (FLOAT)g / 31.0f;
				newPix.b = (FLOAT)b / 31.0f;

				{
					RGBFloat error;
					error.r = oldPix->r - newPix.r;
					error.g = oldPix->g - newPix.g;
					error.b = oldPix->b - newPix.b;

					if (x + 1 < width)
					{
						RGBFloat* pix = &buffer[y][x + 1];
						pix->r += 7.0f / 16.0f * error.r;
						pix->g += 7.0f / 16.0f * error.g;
						pix->b += 7.0f / 16.0f * error.b;
					}

					if (y + 1 < height)
					{
						RGBFloat* pix = &buffer[y + 1][x];
						pix->r += 5.0f / 16.0f * error.r;
						pix->g += 5.0f / 16.0f * error.g;
						pix->b += 5.0f / 16.0f * error.b;

						if (x - 1 >= 0)
						{
							pix = &buffer[y + 1][x - 1];
							pix->r += 3.0f / 16.0f * error.r;
							pix->g += 3.0f / 16.0f * error.g;
							pix->b += 3.0f / 16.0f * error.b;
						}

						if (x + 1 < width)
						{
							pix = &buffer[y + 1][x + 1];
							pix->r += 1.0f / 16.0f * error.r;
							pix->g += 1.0f / 16.0f * error.g;
							pix->b += 1.0f / 16.0f * error.b;
						}
					}
				}
			}
		}
	}
}

namespace Hooks
{
#pragma optimize("s", on)
	VOID Patch_Image(HOOKER hooker)
	{
		PatchCall(hooker, 0x0042B949, ConvertRGBA8888To5551);
		PatchCall(hooker, 0x00440922, ConvertRGBA8888To5551);
	}
#pragma optimize("", on)
}