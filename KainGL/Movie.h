#pragma once
#include "windows.h"

struct MovieSection
{
	DWORD isSecPallete; // 2 - palleted
	DWORD isSecData; // 0-RGBA index / 1-RGB index / 2 not palleted
	DWORD size; // 784 - 16 - some offset / frame start
	DWORD count; // 768 - pallete count / RGB = 256
};

struct MovieHeader
{
	CHAR prefix[4]; // "JAM"
	DWORD frameWidth;
	DWORD frameHeight;
	DWORD frameCount;
	MovieSection frameList[];// frames and palletes
};

VOID Decomress_1(BYTE* src, BYTE* dest, DWORD count)
{
	do
	{
		DWORD result = *(DWORD*)src;
		src += 4;

		DWORD blockSize = 32;
		if (count < 32)
			blockSize = count;

		do
		{
			if (result & 1)
			{
				*dest++ = *src++;
				--count;
			}
			else
			{
				WORD v8 = *(WORD*)src;
				src += 2;

				DWORD sameCount = (v8 >> 11) + 3; // high 5 bits

				count -= sameCount;
				if (count)
				{
					memcpy(dest, dest - 33 - 2048 + (v8 & 0x7FF), sameCount);
					dest += sameCount;
				}
			}

			result >>= 1;
		} while (--blockSize);
	} while (count);
}