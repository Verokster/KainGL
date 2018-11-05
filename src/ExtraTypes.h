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

#pragma once
#include "windows.h"
#include "GLib.h"

struct Rect
{
	DWORD x;
	DWORD y;
	DWORD width;
	DWORD height;
};

struct VecSize
{
	DWORD width;
	DWORD height;
};

struct TexSize
{
	FLOAT width;
	FLOAT height;
};

struct Frame
{
	GLuint id[2];
	Rect rect;
	VecSize vSize;
	TexSize tSize;
};

struct Viewport
{
	BOOL refresh;
	DWORD width;
	DWORD height;
	Rect rectangle;
	POINTFLOAT point;
	POINTFLOAT viewFactor;
	POINTFLOAT clipFactor;
};

struct Resolution {
	DWORD width;
	DWORD height;
	union
	{
		DWORD bpp;
		DWORD index;
	};
};

struct DisplayMode
{
	DWORD dwWidth;
	DWORD dwHeight;
	DWORD dwBPP;
	union
	{
		DWORD dwFrequency;
		BOOL dwExists;
	};
};

struct ShaderProgram
{
	GLuint id;
	DWORD vertexName;
	DWORD fragmentName;
	GLfloat* mvp;
	BOOL interlaced;
};

struct ShaderProgramsList
{
	ShaderProgram nearest;
	ShaderProgram bicubic;
};

struct SubtitlesLine
{
	DWORD startTick;
	DWORD endTick;
	WCHAR* text;
};

enum SubtitlesType
{
	SubtitlesOther,
	SubtitlesVideo,
	SubtitlesWalk,
	SubtitlesInventory,
	SubtitlesDescription
};

struct SubtitlesItem
{
	CHAR* key;
	DWORD count;
	SubtitlesType type;
	DWORD flags;
	SubtitlesLine* lines;
};

struct ALBuffer
{
	DWORD id;
	BOOL loaded;
};

struct ALSoundOptions
{
	BOOL isPositional;
	DWORD waveIndex;
	FLOAT pitch;
	BOOL isAbsolute;
	FLOAT rolloffFactor;
	FLOAT referenceDistance;
	struct {
		FLOAT x;
		FLOAT y;
		FLOAT z;
	} position;
};

struct VibrationOptions
{
	struct {
		FLOAT left;
		FLOAT right;
	} density;
	DWORD start;
	DWORD duration;
};

#ifndef WH_MOUSE_LL
#define WH_MOUSE_LL        14

typedef struct tagMSLLHOOKSTRUCT {
	POINT   pt;
	DWORD   mouseData;
	DWORD   flags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
} MSLLHOOKSTRUCT, FAR *LPMSLLHOOKSTRUCT, *PMSLLHOOKSTRUCT;
#endif