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
#include "Vibration.h"
#include "Config.h"

struct {
	FLOAT left;
	FLOAT right;
} totalValue;

namespace Vibration
{
	VOID Set()
	{
		if (!config.other.forceFeedback)
			return;

		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if (xJoyListConnected[i] && xJoyListCheck[i])
			{
				XINPUT_VIBRATION pVibration =
				{
					WORD(totalValue.left * 65535.0),
					WORD(totalValue.right * 65535.0)
				};
				InputSetState(i, &pVibration);
				break;
			}
		}
	}

	VOID Add(VibrationOptions* options)
	{
		if (options->start)
			return;

		BOOL reset = FALSE;
		{
			if (options->density.left != 0.0f)
			{
				totalValue.left += options->density.left - totalValue.left * options->density.left;
				reset = TRUE;
			}

			if (options->density.right != 0.0f)
			{
				totalValue.right += options->density.right - totalValue.right * options->density.right;
				reset = TRUE;
			}
		}

		if (!reset)
			return;

		options->start = timeGetTime();
		Set();
	}

	VOID Remove(VibrationOptions* options)
	{
		if (!options->start)
			return;

		BOOL reset = FALSE;
		{
			if (options->density.left != 0.0f)
			{
				totalValue.left = options->density.left != 1.0f ? (totalValue.left - options->density.left) / (1.0f - options->density.left) : 0.0f;
				reset = TRUE;
			}

			if (options->density.right != 0.0f)
			{
				totalValue.right = options->density.right != 1.0f ? (totalValue.right - options->density.right) / (1.0f - options->density.right) : 0.0f;
				reset = TRUE;
			}
		}

		if (!reset)
			return;

		options->start = 0;
		Set();
	}
}