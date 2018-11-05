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
#include "FpsCounter.h"

BOOL isFpsChanged;

FpsCounter::FpsCounter(DWORD accuracy)
{
	this->accuracy = accuracy;
	this->fpsQueue = (FLOAT*)MemoryAlloc(accuracy * sizeof(FLOAT));
	this->tickQueue = (DWORD*)MemoryAlloc(accuracy * sizeof(DWORD));
	this->Reset();
}

FpsCounter::~FpsCounter()
{
	MemoryFree(this->fpsQueue);
	MemoryFree(this->tickQueue);
}

VOID FpsCounter::Reset()
{
	this->index = 0;
	this->total = 0;
	this->count = 0;
	this->summary = 0.0f;

	MemoryZero(fpsQueue, this->accuracy * sizeof(FLOAT));
	MemoryZero(tickQueue, this->accuracy * sizeof(DWORD));
}

VOID FpsCounter::Calculate()
{
	DWORD tick = GetTickCount();

	++this->total;
	if (this->count < this->accuracy)
		++this->count;

	DWORD diff = tick - tickQueue[this->total != this->count ? this->index : 0];
	tickQueue[this->index] = tick;

	FLOAT fps = diff ? 1000.0f / diff * this->count : 0.0f;

	FLOAT* queue = &fpsQueue[this->index];
	this->summary -= *queue - fps;
	*queue = fps;

	++this->index;
	if (this->index == this->accuracy)
		this->index = 0;
}

FLOAT FpsCounter::GetValue()
{
	return this->summary / this->count;
}