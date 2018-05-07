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
#include "ddraw.h"
#include "Hooks.h"
#include "Config.h"
#include "DirectDrawSurface.h"

DWORD frameBuffer[320 * 240];
DWORD* palette = (DWORD*)0x008FB570;
LPDIRECTDRAWSURFACE* lpDDSurface = (LPDIRECTDRAWSURFACE*)0x004CD728;

VOID __stdcall CopyFrame(LPVOID src, LPVOID dest, DWORD width, DWORD lPitch, DWORD height, DWORD offset)
{
	memcpy(dest, src, width * height);
}

VOID __stdcall RenderVideoFrame(DWORD* buffers, DWORD index, DWORD width, DWORD height)
{
	DDSURFACEDESC descr;
	descr.dwSize = 108;
	if ((*lpDDSurface)->Lock(NULL, &descr, DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
	{
		DWORD currIndex = index >> 2;
		DWORD interIndex = index & 3;

		if (interIndex == 3)
		{
			BYTE* strData = (BYTE*)*(buffers + (currIndex & 1));
			DWORD* destData1 = (DWORD*)descr.lpSurface;
			DWORD* destData2 = frameBuffer;
			DWORD count = width * height;
			do
			{
				DWORD color = palette[*strData++];
				*destData1++ = color;
				*destData2++ = color;
			} while (--count);
		}
		else if (interIndex == 0)
		{
			DWORD* data1 = frameBuffer;
			BYTE* data2 = (BYTE*)*(buffers + (currIndex & 1));

			BYTE* destData = (BYTE*)descr.lpSurface;

			DWORD count = width * height;
			do
			{
				BYTE* color1 = (BYTE*)data1++;
				BYTE* color2 = (BYTE*)&palette[*data2++];

				*destData++ = BYTE(((DWORD)*color1 * 3 + (DWORD)*color2) >> 2);
				++color1; ++color2;
				*destData++ = BYTE(((DWORD)*color1 * 3 + (DWORD)*color2) >> 2);
				++color1; ++color2;
				*destData = BYTE(((DWORD)*color1 * 3 + (DWORD)*color2) >> 2);

				destData += 2;
			} while (--count);
		}
		else if (interIndex == 1)
		{
			DWORD* data1 = frameBuffer;
			BYTE* data2 = (BYTE*)*(buffers + (currIndex & 1));

			BYTE* destData = (BYTE*)descr.lpSurface;

			DWORD count = width * height;
			do
			{
				BYTE* color1 = (BYTE*)data1++;
				BYTE* color2 = (BYTE*)&palette[*data2++];

				*destData++ = BYTE(((DWORD)*color1 + (DWORD)*color2) >> 1);
				++color1; ++color2;
				*destData++ = BYTE(((DWORD)*color1 + (DWORD)*color2) >> 1);
				++color1; ++color2;
				*destData = BYTE(((DWORD)*color1 + (DWORD)*color2) >> 1);

				destData += 2;
			} while (--count);
		}
		else
		{
			DWORD* data1 = frameBuffer;
			BYTE* data2 = (BYTE*)*(buffers + (currIndex & 1));

			BYTE* destData = (BYTE*)descr.lpSurface;

			DWORD count = width * height;
			do
			{
				BYTE* color1 = (BYTE*)data1++;
				BYTE* color2 = (BYTE*)&palette[*data2++];

				*destData++ = BYTE(((DWORD)*color1 + (DWORD)*color2 * 3) >> 2);
				++color1; ++color2;
				*destData++ = BYTE(((DWORD)*color1 + (DWORD)*color2 * 3) >> 2);
				++color1; ++color2;
				*destData = BYTE(((DWORD)*color1 + (DWORD)*color2 * 3) >> 2);

				destData += 2;
			} while (--count);
		}


		DirectDraw* mdraw = (DirectDraw*)((DirectDrawSurface*)*lpDDSurface)->ddraw;
		mdraw->attachedSurface = (LPDIRECTDRAWSURFACE)*lpDDSurface;
		SetEvent(mdraw->hDrawEvent);
	}
}

DWORD sub_ReadMovieFrameHeader = 0x00468A3C;
DWORD sub_PrepareVideoPallete = 0x004641FC;
DWORD sub_RenderVideoFrame = 0x00461F68;
DWORD back_004640EB = 0x004640EB;

VOID __declspec(naked) hook_00464024()
{
	_asm
	{
		MOV EAX, DWORD PTR SS : [EBP - 0x20] // buffer index
		TEST EAX, EAX // add idx + 3, read and draw first frame
		JNZ checkfour

		ADD EAX, 0x3
		MOV DWORD PTR SS : [EBP - 0x20], EAX
		JMP readfile

		checkfour : MOV ECX, EAX
					AND ECX, 0x3
					TEST ECX, ECX
					JNZ dontpallete_1

					readfile : MOV ECX, DWORD PTR SS : [EBP - 0x30]
							   PUSH ECX
							   SHR EAX, 0x2
							   PUSH EAX
							   CALL sub_ReadMovieFrameHeader
							   ADD ESP, 0x8

							   TEST EAX, EAX
							   JNZ readed
							   MOV DWORD PTR SS : [EBP - 0x28], 0x1
							   JMP increment

							   readed : MOV DWORD PTR SS : [EBP - 0x0C], 0x1

										CALL DWORD PTR SS : [EBP + 0x28]
										TEST EAX, EAX
										JZ aftercheck_1
										MOV DWORD PTR DS : [0x004CD744], -0x1
										MOV DWORD PTR SS : [EBP - 0x38], 0x1
										MOV EAX, DWORD PTR SS : [EBP - 0x38]
										MOV DWORD PTR SS : [EBP - 0x28], EAX

										aftercheck_1 : MOV EAX, 0x0097CCF0
													   PUSH EAX
													   CALL sub_PrepareVideoPallete
													   ADD ESP, 0x4
													   JMP dontpallete_2

													   dontpallete_1 : CALL DWORD PTR SS : [EBP + 0x28]
																	   TEST EAX, EAX
																	   JZ aftercheck_1
																	   MOV DWORD PTR DS : [0x004CD744], -0x1
																	   MOV DWORD PTR SS : [EBP - 0x38], 0x1
																	   MOV EAX, DWORD PTR SS : [EBP - 0x38]
																	   MOV DWORD PTR SS : [EBP - 0x28], EAX

																	   dontpallete_2 : CALL DWORD PTR SS : [EBP + 0x28]
																					   TEST EAX, EAX
																					   JZ aftercheck_2
																					   MOV DWORD PTR DS : [0x004CD744], -0x1
																					   MOV DWORD PTR SS : [EBP - 0x3C], 0x1
																					   MOV EAX, DWORD PTR SS : [EBP - 0x3C]
																					   MOV DWORD PTR SS : [EBP - 0x28], EAX

																					   aftercheck_2 : MOV EAX, DWORD PTR SS : [EBP - 0x14]
																									  PUSH EAX
																									  MOV EAX, DWORD PTR SS : [EBP - 0x18]
																									  PUSH EAX
																									  MOV EAX, DWORD PTR SS : [EBP - 0x20] // buffer index
																									  PUSH EAX
																									  MOV EAX, DWORD PTR SS : [EBP - 0x30] // buffer pointer
																									  PUSH EAX
																									  CALL RenderVideoFrame

																									  CALL DWORD PTR SS : [EBP + 0x28]
																									  TEST EAX, EAX
																									  JZ increment
																									  MOV DWORD PTR DS : [0x004CD744], -0x1
																									  MOV DWORD PTR SS : [EBP - 0x40], 0x1
																									  MOV EAX, DWORD PTR SS : [EBP - 0x40]
																									  MOV DWORD PTR SS : [EBP - 0x28], EAX

																									  increment : MOV EAX, DWORD PTR SS : [EBP - 0x20]
																												  INC DWORD PTR SS : [EBP - 0x20]
																												  JMP back_004640EB

	}
}

VOID ClearMovieDisplay()
{
	DDSURFACEDESC desc;
	desc.dwSize = sizeof(DDSURFACEDESC);

	LPDIRECTDRAWSURFACE* lpDDSurface = (LPDIRECTDRAWSURFACE*)0x004CD728;
	if ((*lpDDSurface)->Lock(0, &desc, 0, NULL) == DD_OK)
	{
		memset(desc.lpSurface, NULL, desc.lPitch * desc.dwHeight);
		(*lpDDSurface)->Unlock(NULL);
	}
}

VOID __declspec(naked) hook_0046418C()
{
	_asm
	{
		JMP	ClearMovieDisplay
	}
}

namespace Hooks
{
	VOID Patch_Video()
	{
		PatchDWord(0x004CD73C, 320);
		PatchDWord(0x004CD740, 240);

		if (configVideoSkipIntro)
		{
			// skip movies
			PatchNop(0x0042A6FD, 0x0042A72A - 0x0042A6FD); // Skip movies
			PatchNop(0x0042A754, 0x0042A7E5 - 0x0042A754); // Skip intro
		}
		else
		{
			PatchDWord(0x0042A7A5 + 1, 4000); // Lower intro 1 timeout
			PatchDWord(0x0042A7D9 + 1, 3000); // Lower intro 2 timeout
		}

		if (configVideoSmoother)
		{
			PatchByte(0x00463EDD + 1, 32);

			PatchByte(0x00444870 + 1, 60);
			PatchHook(0x00464024, hook_00464024);

			// Clear screen
			PatchHook(0x0046418C, hook_0046418C);
		}
		else
		{
			PatchInt(0x00461FDA + 1, ((INT)CopyFrame - 0x00461FDF));
			PatchNop(0x00461FDF, 3);
		}
	}
}