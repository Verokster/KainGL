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
#include "ShaderSource.h"

const GLchar* shVertexSoure =
"#version 130\n"
"precision lowp float;"
"uniform mat4 mvp;"
"in vec4 vCoord;"
"in vec2 vTexCoord;"
"out vec2 fTexCoord;"
"void main() {"
	"gl_Position = mvp * vec4(vCoord.xy, 0.0, 1.0);"
	"fTexCoord = vTexCoord;"
"}";

const GLchar* shFragmentFullscreen =
"#version 130\n"
"precision lowp float;"
"uniform sampler2D tex01;"
"uniform sampler1D pal01;"
"in vec2 fTexCoord;"
"out vec4 fragColor;"
"void main(void) {"
	"float index = texture2D(tex01, fTexCoord).x;"
	"fragColor = texture1D(pal01, index);"
"}";

const GLchar* shFragmentWindowed = 
"#version 130\n"
"precision lowp float;"
"uniform sampler2D tex01;"
"in vec2 fTexCoord;"
"out vec4 fragColor;"
"void main(void) {"
	"fragColor = texture2D(tex01, fTexCoord);"
"}";