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

#define LIST_LENGTH 353
#define STR_LENGTH 430
#define INSERT_INDEX 6

WCHAR* tableLarge;
WCHAR* tableSmall;
WCHAR* tableLoadSave;
BYTE* indents;

DWORD __fastcall DecodeLine(BYTE* ptr, CHAR* dst, WCHAR* table)
{
	DWORD dataLength = 0;
	DWORD strLength;
	WCHAR* line = DecodeUtf8(ptr, &dataLength, &strLength);
	if (line)
	{
		for (DWORD i = 0; i < strLength; ++i)
		{
			WCHAR ch = line[i];
			if (ch == ' ')
			{
				dst[i] = ' ';
				continue;
			}

			for (int j = 0; j < 90; ++j)
			{
				if (table[j] == ch)
				{
					dst[i] = j + '!';
					break;
				}
			}
		}
		dst[strLength] = NULL;
		MemoryFree(line);
	}

	return dataLength;
}

VOID __fastcall DecodeLine(CHAR* str, WCHAR* table)
{
	while (*str)
	{
		WCHAR wch = *str;
		if (wch != ' ')
		{
			for (int j = 0; j < 90; ++j)
			{
				if (table[j] == wch)
				{
					*str = j + '!';
					break;
				}
			}
		}

		++str;
	}
}

VOID __cdecl PrintPage(CHAR* dst, const CHAR* format, DWORD page)
{
	CHAR line[20];
	StrPrint(line, "%d", page);
	DecodeLine(line, tableLoadSave);
	
	StrCopy(dst, format);
	StrCat(dst, " ");
	StrCat(dst, line);
}

VOID __cdecl PrintSave(CHAR* dst, const CHAR* format, DWORD page, DWORD save)
{
	StrPrint(dst, format, page, save);
	DecodeLine(dst, tableLoadSave);
}

namespace Hooks
{
	VOID Patch_Credits(HOOKER hooker)
	{
		BYTE* creditsList = NULL;
		{
			DWORD hash = ((DWORD(__cdecl*)(CHAR*))Hooks::sub_GetHash)("GAME/LOCALE.STR");

			FileHeader* file = filesHeaders;
			DWORD ddd = 0;
			DWORD count = *filesCount;
			while (count--)
			{
				if (file->hash == hash && file->size > 6)
				{
					DWORD index = *filesCount - count - 1;

					FILE* stream = filesHandlers[index];

					if (!stream)
					{
						CHAR* filePath = bigPathes[index];
						stream = FileOpen(filePath, "rb");
					}

					if (stream && !FileSeek(stream, file->offset, SEEK_SET))
					{
						BYTE* buffer = (BYTE*)MemoryAlloc(file->size);
						if (buffer)
						{
							if (FileRead(buffer, file->size, 1, stream) && !StrCompare((CHAR*)buffer, "STR"))
							{
								BYTE* ptr = buffer + sizeof(DWORD);

								indents = (BYTE*)MemoryAlloc(90 * 3);

								DWORD dataLength = (DWORD)*(WORD*)ptr;
								ptr += sizeof(WORD);
								tableLarge = DecodeUtf8(ptr, &dataLength);
								ptr += dataLength;
								MemoryCopy(indents, ptr, 90);
								ptr += 90;

								dataLength = (DWORD)*(WORD*)ptr;
								ptr += sizeof(WORD);
								tableSmall = DecodeUtf8(ptr, &dataLength);
								ptr += dataLength;
								MemoryCopy(indents + 90, ptr, 90);
								ptr += 90;

								dataLength = (DWORD)*(WORD*)ptr;
								ptr += sizeof(WORD);
								tableLoadSave = DecodeUtf8(ptr, &dataLength);
								ptr += dataLength;
								MemoryCopy(indents + 180, ptr, 90);
								ptr += 90;

								PatchDWord(hooker, 0x00419DB7 + 1, 90);
								PatchDWord(hooker, 0x00419DA3 + 1, 180);
								PatchDWord(hooker, 0x0041A471 + 1, 90);

								PatchDWord(hooker, 0x00419E74 + 2, (DWORD)indents - '!');
								PatchDWord(hooker, 0x00419E94 + 2, (DWORD)indents - '!');
								PatchDWord(hooker, 0x0041A48C + 3, (DWORD)indents - '!');

								DWORD lines = (DWORD)*(WORD*)ptr;
								ptr += sizeof(WORD);

								Strings* stringsList = (Strings*)MemoryAlloc(sizeof(Strings));
								if (stringsList)
								{
									stringsList->strYouAre.table = tableLarge;
									stringsList->strVictorious.table = tableLarge;
									stringsList->strYouHave.table = tableLarge;
									stringsList->strPerished.table = tableLarge;
									stringsList->strSlayings.table = tableLarge;
									stringsList->strMeals.table = tableLarge;
									stringsList->strMutilations.table = tableLarge;
									stringsList->strSecrets.table = tableLarge;
									stringsList->strPrestige.table = tableLarge;

									stringsList->strWhelp.table = tableSmall;
									stringsList->strGimp.table = tableSmall;
									stringsList->strPrincess.table = tableSmall;
									stringsList->strBride.table = tableSmall;
									stringsList->strPrince.table = tableSmall;
									stringsList->strBloodHunter.table = tableSmall;
									stringsList->strCount.table = tableSmall;
									stringsList->strBaron.table = tableSmall;
									stringsList->strOverloard.table = tableSmall;
									stringsList->strSaint.table = tableSmall;
									stringsList->strDevourerOfWorlds.table = tableSmall;

									stringsList->strPage.table = tableLoadSave;

									StringsItem* item = (StringsItem*)stringsList;
									do
									{
										ptr += DecodeLine(ptr, item->text, item->table) + 1;
										++item;
									} while (--lines);

									{
										DWORD address = 0x0041973C;
										PatchPtr(hooker, address, stringsList->strWhelp.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strGimp.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strPrincess.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strBride.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strPrince.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strBloodHunter.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strCount.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strBaron.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strOverloard.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strSaint.text);
										address += 4;
										PatchPtr(hooker, address, stringsList->strDevourerOfWorlds.text);

										address = 0x00496A67;
										PatchBlock(hooker, address, stringsList->strYouHave.text, STRINGS_SIZE);
										address += STRINGS_SIZE + 1;
										PatchBlock(hooker, address, stringsList->strPerished.text, STRINGS_SIZE);
										address += (STRINGS_SIZE + 1) << 1;
										PatchBlock(hooker, address, stringsList->strSlayings.text, STRINGS_SIZE);
										address += (STRINGS_SIZE + 1) << 1;
										PatchBlock(hooker, address, stringsList->strMeals.text, STRINGS_SIZE);
										address += (STRINGS_SIZE + 1) << 1;
										PatchBlock(hooker, address, stringsList->strMutilations.text, STRINGS_SIZE);
										address += (STRINGS_SIZE + 1) << 1;
										PatchBlock(hooker, address, stringsList->strSecrets.text, STRINGS_SIZE);
										address += (STRINGS_SIZE + 1) << 1;
										PatchBlock(hooker, address, stringsList->strPrestige.text, STRINGS_SIZE);


										PatchPtr(hooker, 0x0041A2DE + 1, stringsList->strYouAre.text);
										PatchPtr(hooker, 0x0041A331 + 1, stringsList->strYouAre.text);

										PatchPtr(hooker, 0x0041A379 + 1, stringsList->strYouHave.text);

										PatchPtr(hooker, 0x0041A2F0 + 1, stringsList->strVictorious.text);
										PatchPtr(hooker, 0x0041A343 + 1, stringsList->strVictorious.text);
										
										PatchPtr(hooker, 0x0041A38B + 1, stringsList->strPerished.text);

										PatchPtr(hooker, 0x0041A707 + 1, stringsList->strPage.text);

										PatchCall(hooker, 0x0041A711, PrintPage);

										PatchCall(hooker, 0x0043DFB7, PrintSave);
									}

									DWORD lines = (DWORD)*(WORD*)ptr;
									ptr += sizeof(WORD);

									creditsList = (BYTE*)MemoryAlloc((lines + 1) * STR_LENGTH);
									{
										CHAR* str = (CHAR*)creditsList;
										StrCopy((CHAR*)(creditsList + lines * STR_LENGTH), "END");

										do
										{
											CHAR ch = *ptr++;
											*str++ = ch;

											ptr += DecodeLine(ptr, str, ch == '0' ? tableSmall : tableLarge) + 1;
											str += STR_LENGTH - 1;
										} while (--lines);
									}
								}
							}

							MemoryFree(buffer);
						}
					}

					break;
				}

				++file;
				++ddd;
			}
		}

		if (!creditsList)
		{
			creditsList = (BYTE*)MemoryAlloc((LIST_LENGTH + 4) * STR_LENGTH);

			DWORD idx = INSERT_INDEX;

			DWORD baseOffset = GetBaseOffset(hooker);
			BYTE* originalList = (BYTE*)f(0x00496C0A);
			MemoryCopy(creditsList, originalList, idx * STR_LENGTH);

			StrCopy((CHAR*)&creditsList[idx++ * STR_LENGTH], "1GL WRAPPER & PATCH");
			StrCopy((CHAR*)&creditsList[idx++ * STR_LENGTH], "1DEVELOPED BY");
			StrCopy((CHAR*)&creditsList[idx++ * STR_LENGTH], "0OLEKSIY RYABCHUN");
			StrCopy((CHAR*)&creditsList[idx++ * STR_LENGTH], "0(VEROK)");

			MemoryCopy(&creditsList[idx * STR_LENGTH], originalList + INSERT_INDEX * STR_LENGTH, LIST_LENGTH * STR_LENGTH - INSERT_INDEX * STR_LENGTH);
		}

		PatchPtr(hooker, 0x00419826 + 2, creditsList);
		PatchPtr(hooker, 0x004199AC + 2, creditsList);
		PatchPtr(hooker, 0x004199DE + 2, creditsList);
		PatchPtr(hooker, 0x004199EF + 2, creditsList);
		PatchPtr(hooker, 0x00419A0D + 2, creditsList);
		PatchPtr(hooker, 0x00419A20 + 2, creditsList);
		PatchPtr(hooker, 0x00419A36 + 2, creditsList);

		PatchPtr(hooker, 0x00419C15 + 1, creditsList);

		PatchPtr(hooker, 0x00419C6A + 1, creditsList);
		PatchPtr(hooker, 0x00419CA8 + 1, creditsList);
		PatchPtr(hooker, 0x00419CEC + 3, creditsList);
		PatchPtr(hooker, 0x00419CF7 + 1, creditsList);

		PatchByte(hooker, 0x0041990C + 2, 24); // decrease speed 
	}
}