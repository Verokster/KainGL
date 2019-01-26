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
#include "ddraw.h"
#include "Hooks.h"
#include "Main.h"
#include "Config.h"
#include "OpenDrawSurface.h"
#include "OpenDraw.h"

#define TOP_FIRST TRUE

BOOL interlaced;
DWORD frameBuffer[2][320][240];
DWORD floatBuffer1[324][240];
DWORD floatBuffer2[323][240][3];
DWORD* palette = (DWORD*)0x008FB570;
LPDIRECTDRAWSURFACE* lpDDSurface = (LPDIRECTDRAWSURFACE*)0x004CD728;

VOID __cdecl CopyFrame(LPVOID src, LPVOID dest, DWORD width, DWORD lPitch, DWORD height, DWORD offset)
{
	MemoryCopy(dest, src, width * height);
}

INT __inline Cubic1(INT p0, INT p1, INT p2, INT p3)
{
	return (p1 + p2) * 9 - p0 - p3;
}

BYTE __inline Cubic2(INT p0, INT p1, INT p2, INT p3)
{
	INT res = Cubic1(p0, p1, p2, p3) >> 8;

	if (res < 0)
		res = 0;
	else if (res > 255)
		res = 255;

	return (BYTE)res;
}

VOID __stdcall RenderVideoFrame(DWORD* buffers, DWORD index, DWORD width, DWORD height)
{
	DDSURFACEDESC descr;
	descr.dwSize = 108;

	DWORD currIndex = index >> 2;
	DWORD interIndex = index & 3;

	DWORD bufferIndex = ((index + 1) >> 2) & 1;
	DWORD count = width * height;

	if (interlaced)
	{
		if (!interIndex || !(index - 3))
		{
			DWORD* dst = (DWORD*)frameBuffer[interIndex & 1 ? bufferIndex : bufferIndex ^ 1];
			BYTE* src = (BYTE*)*(buffers + bufferIndex);
			do
				*dst++ = palette[*src++];
			while (--count);
		}

		if ((interIndex & 1) && (*lpDDSurface)->Lock(NULL, &descr, DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
		{
			DWORD step = width * 3;
			DWORD pitch = width << 2;

			// Collect data
			{
				BYTE* srcTop = (BYTE*)frameBuffer[bufferIndex];
				BYTE* srcBottom = (BYTE*)frameBuffer[interIndex & 2 ? bufferIndex ^ 1 : bufferIndex] + pitch;

				DWORD* dstTop = (DWORD*)floatBuffer1 + (width << 1);
				DWORD* dstBottom = dstTop + width;

				DWORD ch = height >> 1;
				do
				{
					MemoryCopy(dstTop, srcTop, pitch);
					MemoryCopy(dstBottom, srcBottom, pitch);

					srcTop += pitch << 1;
					srcBottom += pitch << 1;

					dstTop += width << 1;
					dstBottom += width << 1;
				} while (--ch);
			}

			// Cubic interpolation - STEP 1
			{
				BYTE* src0 = (BYTE*)floatBuffer1;
				BYTE* src1 = src0 + pitch;
				BYTE* src2 = src1 + pitch;
				BYTE* src3 = src2 + pitch;

				INT* dst = (INT*)floatBuffer2 + step;

				DWORD size = (height + 1) * width;
				do
				{
					DWORD count = 3;
					do
						*dst++ = Cubic1((INT)*src0++, (INT)*src1++, (INT)*src2++, (INT)*src3++);
					while (--count);

					++src0; ++src1; ++src2; ++src3;
				} while (--size);
			}

			// Cubic interpolation - STEP 2
			{
				INT* src0 = (INT*)floatBuffer2;
				INT* src1 = src0 + step;
				INT* src2 = src1 + step;
				INT* src3 = src2 + step;

				BYTE* dst = (BYTE*)descr.lpSurface;
				
				DWORD ch = height;
				do
				{
					BYTE* dstVal = dst;
					
					DWORD cw = width;
					do
					{
						DWORD count = 3;
						do
							*dstVal++ = Cubic2(*src0++, *src1++, *src2++, *src3++);
						while (--count);

						++dstVal;
					} while (--cw);

					dst += pitch;
				} while (--ch);
			}

			(*lpDDSurface)->Unlock((LPVOID)1);
		}
	}
	else if ((*lpDDSurface)->Lock(NULL, &descr, DDLOCK_SURFACEMEMORYPTR, NULL) == DD_OK)
	{
		if (interIndex == 3)
		{
			if (!currIndex)
			{
				DWORD* data1 = (DWORD*)frameBuffer[bufferIndex];
				BYTE* src = (BYTE*)*(buffers + bufferIndex);
				DWORD* destData = (DWORD*)descr.lpSurface;
				do
				{
					DWORD color = palette[*src++];
					*destData++ = color;
					*data1++ = color;
				} while (--count);
			}
			else
				MemoryCopy(descr.lpSurface, frameBuffer[bufferIndex], count);
		}
		else
		{
			if (interIndex == 0)
			{
				BYTE* destData = (BYTE*)descr.lpSurface;
				BYTE* color1 = (BYTE*)frameBuffer[bufferIndex];
				DWORD* data2 = (DWORD*)frameBuffer[bufferIndex ^ 1];
				BYTE* src = (BYTE*)*(buffers + bufferIndex);

				do
				{
					DWORD color = palette[*src++];
					*data2++ = color;
					BYTE* color2 = (BYTE*)&color;

					DWORD count = 3;
					do
						*destData++ = BYTE(((DWORD)*color1++ * 3 + (DWORD)*color2++) >> 2);
					while (--count);

					++destData; ++color1;
				} while (--count);
			}
			else if (interIndex == 1)
			{
				BYTE* destData = (BYTE*)descr.lpSurface;
				BYTE* color1 = (BYTE*)frameBuffer[bufferIndex];
				BYTE* color2 = (BYTE*)frameBuffer[bufferIndex ^ 1];

				do
				{
					DWORD count = 3;
					do
						*destData++ = BYTE(((DWORD)*color1++ + (DWORD)*color2++) >> 1);
					while (--count);

					++destData; ++color1; ++color2;
				} while (--count);
			}
			else
			{
				BYTE* destData = (BYTE*)descr.lpSurface;
				BYTE* color1 = (BYTE*)frameBuffer[bufferIndex];
				BYTE* color2 = (BYTE*)frameBuffer[bufferIndex ^ 1];

				do
				{
					DWORD count = 3;
					do
						*destData++ = BYTE(((DWORD)*color1++ + (DWORD)*color2++ * 3) >> 2);
					while (--count);

					++destData; ++color1; ++color2;
				} while (--count);
			}
		}

		(*lpDDSurface)->Unlock((LPVOID)1);
	}
}

DWORD sub_ReadMovieFrameHeader = 0x00468A3C;
DWORD sub_PrepareVideoPallete = 0x004641FC;
DWORD sub_RenderVideoFrame = 0x00461F68;
DWORD some_004CD744 = 0x004CD744;
DWORD some_0097CCF0 = 0x0097CCF0;
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
										MOV EAX, some_004CD744
										MOV DWORD PTR DS : [EAX], -0x1
										MOV DWORD PTR SS : [EBP - 0x38], 0x1
										MOV EAX, DWORD PTR SS : [EBP - 0x38]
										MOV DWORD PTR SS : [EBP - 0x28], EAX

										aftercheck_1 : MOV EAX, some_0097CCF0
													   PUSH EAX
													   CALL sub_PrepareVideoPallete
													   ADD ESP, 0x4
													   JMP dontpallete_2

													   dontpallete_1 : CALL DWORD PTR SS : [EBP + 0x28]
																	   TEST EAX, EAX
																	   JZ aftercheck_1
																	   MOV EAX, some_004CD744
																	   MOV DWORD PTR DS : [EAX], -0x1
																	   MOV DWORD PTR SS : [EBP - 0x38], 0x1
																	   MOV EAX, DWORD PTR SS : [EBP - 0x38]
																	   MOV DWORD PTR SS : [EBP - 0x28], EAX

																	   dontpallete_2 : CALL DWORD PTR SS : [EBP + 0x28]
																					   TEST EAX, EAX
																					   JZ aftercheck_2
																					   MOV EAX, some_004CD744
																					   MOV DWORD PTR DS : [EAX], -0x1
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
																									  MOV EAX, some_004CD744
																									  MOV DWORD PTR DS : [EAX], -0x1
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
	MemoryZero(frameBuffer, sizeof(frameBuffer));

	DDSURFACEDESC desc;
	desc.dwSize = sizeof(DDSURFACEDESC);

	if ((*lpDDSurface)->Lock(0, &desc, 0, NULL) == DD_OK)
	{
		MemoryZero(desc.lpSurface, desc.lPitch * desc.dwHeight);
		(*lpDDSurface)->Unlock((LPVOID)1);
	}
}

DWORD checkTick;
VOID ClearInputState()
{
	MemoryZero((VOID*)(0x008F8FB0 + Hooks::baseOffset), 1024); // clear keys
	MemoryZero((VOID*)(0x004C8FBC + Hooks::baseOffset), 12); // clear mouse
	MemoryZero((VOID*)(0x0058CE38 + Hooks::baseOffset), 32); // clear gamepad

	checkTick = GetTickCount();

	if (configVideoSmoother)
		ClearMovieDisplay();
}

BOOL __stdcall CheckInputState(BOOL check)
{
	if (check)
	{
		DWORD newTick = GetTickCount();
		if (newTick > checkTick + 700)
		{
			checkTick = newTick;
			return TRUE;
		}
	}

	return FALSE;
}

VOID __declspec(naked) hook_00444EE2()
{
	_asm
	{
		MOV EAX, EBX
		POP ESI
		POP EBX

		POP ECX
		PUSH EAX
		PUSH ECX
		JMP CheckInputState
	}
}

DWORD sub_ProcessMessage = 0x00464404;
DWORD sub_CalcTimeout = 0x00444D44;

VOID __declspec(naked) IntroTimeout()
{
	_asm
	{
		PUSH Hooks::hMainWnd
		CALL sub_ProcessMessage
		ADD ESP, 0x4
		JMP	sub_CalcTimeout
	}
}

VOID __cdecl CheckVideoFile(CHAR* dest, const CHAR* format, CHAR* path, CHAR* name)
{
	StrPrint(dest, format, kainDirPath, name);

	interlaced = name == Hooks::trailersList[0] || name == Hooks::trailersList[1];

	FILE* hFile = FileOpen(dest, "rb");
	if (hFile)
		FileClose(hFile);
	else
		StrPrint(dest, format, path, name);
}

namespace Hooks
{
	VOID Patch_Video()
	{
		palette = (DWORD*)((DWORD)palette + baseOffset);
		lpDDSurface = (LPDIRECTDRAWSURFACE*)((DWORD)lpDDSurface + baseOffset);

		PatchDWord(0x004CD73C, 320);
		PatchDWord(0x004CD740, 240);

		// Remove delay start playing
		PatchNop(0x0045198B, 2);
		PatchJump(0x004640EB, 0x0046412F + baseOffset);
		PatchHook(0x00444EE2, hook_00444EE2);

		if (configVideoSkipIntro)
		{
			PatchNop(0x0042A6FD, 0x0042A72A - 0x0042A6FD); // Skip movies
			PatchNop(0x0042A754, 0x0042A7E5 - 0x0042A754); // Skip intro
		}
		else
		{
			PatchDWord(0x0042A7A5 + 1, 4000); // Lower intro 1 timeout
			PatchDWord(0x0042A7D9 + 1, 3000); // Lower intro 2 timeout

			sub_ProcessMessage += baseOffset;
			sub_CalcTimeout += baseOffset;

			PatchCall(0x0042A79D, IntroTimeout);
			PatchCall(0x0042A7D1, IntroTimeout);
		}

		// Clear screen and input state before playing 
		PatchCall(0x00463F2C, ClearInputState);
		PatchCall(0x00464147, ClearInputState);

		if (configVideoSmoother)
		{
			PatchByte(0x00463EDD + 1, 32);

			PatchByte(0x00444870 + 1, 60);
			PatchHook(0x00464024, hook_00464024);

			sub_ReadMovieFrameHeader += baseOffset;
			sub_PrepareVideoPallete += baseOffset;
			sub_RenderVideoFrame += baseOffset;
			some_004CD744 += baseOffset;
			some_0097CCF0 += baseOffset;
			back_004640EB += baseOffset;
		}
		else
			PatchCall(0x00461FDA, CopyFrame);

		PatchCall(0x004519A4, CheckVideoFile);
	}
}