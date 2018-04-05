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
#include "stdio.h"
#include "math.h"
#include "DirectDraw.h"

DirectDraw* ddrawList;

namespace Main
{
	HRESULT WINAPI DirectDrawCreate(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown * pUnkOuter)
	{
		ddrawList = new DirectDraw(ddrawList);
		*(DirectDraw**)lplpDD = ddrawList;
		return DD_OK;
	}

	HRESULT WINAPI DirectDrawEnumerate(LPDDENUMCALLBACK lpCallback, LPVOID lpContext)
	{
		GUID id = { 0X51221AA6, 0XC5DA, 0X468F, 0X82, 0X31, 0X68, 0X0E, 0XC9, 0X03, 0XA3, 0XB8 };
		lpCallback(&id, "OpenGL Wrapper", "OpenGL Wrapper", lpContext);
		return DD_OK;
	}

	DirectDraw* FindDirectDrawByWindow(HWND hWnd)
	{
		DirectDraw* ddraw = ddrawList;
		while (ddraw)
		{
			if (ddraw->hWnd == hWnd)
				return ddraw;

			ddraw = ddraw->last;
		}

		return NULL;
	}

	DWORD __fastcall Round(FLOAT number)
	{
		FLOAT floorVal = floor(number);
		return DWORD(floorVal + 0.5f > number ? floorVal : ceil(number));
	}

	VOID __fastcall ShowError(CHAR* message, CHAR* file, DWORD line)
	{
		CHAR dest[400];
		sprintf(dest, "%s\n\n\nFILE %s\nLINE %d", message, file, line);
		MessageBox(NULL, dest, "Error", MB_OK | MB_ICONERROR);
	}

#ifdef _DEBUG
	VOID __fastcall CheckError(CHAR* file, DWORD line)
	{
		return;
		DWORD statusCode = GLGetError();

		CHAR* message;

		if (statusCode != GL_NO_ERROR)
		{
			switch (statusCode)
			{
			case GL_INVALID_ENUM:
				message = "GL_INVALID_ENUM";
				break;

			case GL_INVALID_VALUE:
				message = "GL_INVALID_VALUE";
				break;

			case GL_INVALID_OPERATION:
				message = "GL_INVALID_OPERATION";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				message = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;

			case GL_OUT_OF_MEMORY:
				message = "GL_OUT_OF_MEMORY";
				break;

			case GL_STACK_UNDERFLOW:
				message = "GL_STACK_UNDERFLOW";
				break;

			case GL_STACK_OVERFLOW:
				message = "GL_STACK_OVERFLOW";
				break;

			default:
				message = "GL_UNDEFINED";
				break;
			}

			ShowError(message, file, line);
		}
	}
#endif
}