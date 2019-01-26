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
	DWORD dwFrequency;
	BOOL dwExists;
};

struct ShaderProgram
{
	GLuint id;
	DWORD vertexName;
	DWORD fragmentName;
	GLfloat* mvp;
	BOOL interlaced;
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
	CHAR key[16];
	DWORD count;
	SubtitlesType type;
	DWORD flags;
	SubtitlesLine* lines;
};

struct SubtitlesFont
{
	HFONT font;
	COLORREF color;
	COLORREF background;
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

struct SceneData
{
	BOOL isVSync;
};

struct SceneDataOld : SceneData
{
	GLuint scalelineId;
	Frame* frames;
	DWORD frameCount;
	VOID* pixelBuffer;
};

struct SceneDataNew : SceneData
{
	GLuint arrayName;
	GLuint bufferName;
	FLOAT buffer[4][4];
	FLOAT mvpMatrix[4][4];
	struct {
		ShaderProgram nearest;
		ShaderProgram bicubic;
	} shaders;
	struct {
		GLuint normalId;
		GLuint scalelineId;
	} textures;
};

#define STRINGS_SIZE 29

struct StringsItem
{
	WCHAR* table;
	CHAR text[STRINGS_SIZE];
};

struct Strings
{
	StringsItem strYouAre;
	StringsItem strVictorious;
	StringsItem strYouHave;
	StringsItem strPerished;
	StringsItem strSlayings;
	StringsItem strMeals;
	StringsItem strMutilations;
	StringsItem strSecrets;
	StringsItem strPrestige;
	StringsItem strWhelp;
	StringsItem strGimp;
	StringsItem strPrincess;
	StringsItem strBride;
	StringsItem strPrince;
	StringsItem strBloodHunter;
	StringsItem strCount;
	StringsItem strBaron;
	StringsItem strOverloard;
	StringsItem strSaint;
	StringsItem strDevourerOfWorlds;
	StringsItem strPage;
};