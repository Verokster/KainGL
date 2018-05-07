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
#include "resource.h"
#include "math.h"
#include "Windowsx.h"
#include "DirectDraw.h"
#include "DirectDrawSurface.h"
#include "ShaderSource.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Main.h"

#pragma region Not Implemented
ULONG DirectDraw::AddRef() { return 0; }
HRESULT DirectDraw::Compact() { return DD_OK; }
HRESULT DirectDraw::EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT DirectDraw::GetDisplayMode(LPDDSURFACEDESC) { return DD_OK; }
HRESULT DirectDraw::GetFourCCCodes(LPDWORD, LPDWORD) { return DD_OK; }
HRESULT DirectDraw::GetGDISurface(LPDIRECTDRAWSURFACE *) { return DD_OK; }
HRESULT DirectDraw::GetMonitorFrequency(LPDWORD) { return DD_OK; }
HRESULT DirectDraw::GetScanLine(LPDWORD) { return DD_OK; }
HRESULT DirectDraw::GetVerticalBlankStatus(LPBOOL) { return DD_OK; }
HRESULT DirectDraw::Initialize(GUID *) { return DD_OK; }
HRESULT DirectDraw::DuplicateSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE *) { return DD_OK; }
HRESULT DirectDraw::QueryInterface(REFIID riid, LPVOID* ppvObj) { return DD_OK; }
#pragma endregion

#define WIN_STYLE WS_POPUPWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE | WS_CLIPSIBLINGS | WS_SIZEBOX
#define FS_STYLE WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_CLIPSIBLINGS
#define VK_I 0x49
#define VK_F 0x46

DisplayMode modesList[9];

WNDPROC OldWindowProc;
LPARAM mousePos;
HWND mousehWnd;

LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
		if (ddraw && ddraw->virtualMode)
		{
			ddraw->viewport.width = LOWORD(lParam);
			ddraw->viewport.height = HIWORD(lParam) - (!configDisplayWindowed ? 1 : 0);
			ddraw->viewport.refresh = TRUE;
			SetEvent(ddraw->hDrawEvent);
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_DISPLAYCHANGE:
	{
		DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
		if (ddraw)
		{
			DEVMODE devMode = { NULL };
			devMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
			ddraw->frequency = devMode.dmDisplayFrequency;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_NCACTIVATE:
	{
		if (!configDisplayWindowed)
		{
			DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
			if (ddraw && ddraw->virtualMode)
			{
				if ((BOOL)wParam)
					ddraw->SetFullscreenMode();
				else if (!ddraw->isFinish)
				{
					ChangeDisplaySettings(NULL, NULL);
					ShowWindow(hWnd, SW_MINIMIZE);
				}
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		if ((HIWORD(lParam) & KF_ALTDOWN))
		{
			switch (wParam)
			{
				// Windowed mode on/off
			case VK_RETURN:
			{
				configDisplayWindowed = !configDisplayWindowed;
				Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_WINDOWED, configDisplayWindowed);

				DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
				if (ddraw)
				{
					if (!configDisplayWindowed)
					{
						if (ddraw->realMode)
						{
							if (ddraw->isStylesLoaded)
								GetWindowPlacement(hWnd, &ddraw->windowPlacement);
							ddraw->SetFullscreenMode();
						}
					}
					else
					{
						if (ddraw->virtualMode)
						{
							ChangeDisplaySettings(NULL, NULL);
							ddraw->SetWindowedMode();
						}
					}

					ddraw->isStateChanged = TRUE;

					ddraw->CalcView();
					ddraw->ScaleMouseOut(&mousePos);
					POINT point = { GET_X_LPARAM(mousePos), GET_Y_LPARAM(mousePos) };
					ClientToScreen(ddraw->hWnd, &point);
					if (!configDisplayWindowed)
						++point.y;
					SetCursorPos(point.x, point.y);
				}

				return NULL;
			}

			// FPS counter on/off
			case VK_I:
			{
				configFpsCounter = !configFpsCounter;
				Config::Set(CONFIG_FPS, CONFIG_FPS_COUNTER, configFpsCounter);
				isFpsChanged = TRUE;
				return NULL;
			}

			// Filtering on/off
			case VK_F:
			{
				configGlFiltering = configGlFiltering == GL_LINEAR ? GL_NEAREST : GL_LINEAR;
				Config::Set(CONFIG_GL, CONFIG_GL_FILTERING, configGlFiltering == GL_LINEAR);

				DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
				if (ddraw)
					ddraw->isStateChanged = TRUE;
				return NULL;
			}

			default:
				break;
			}
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SYSCOMMAND:
	{
		if (wParam == SC_KEYMENU && (lParam >> 16) <= 0)
			break;

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	{
		while (ShowCursor(FALSE) >= -1);

		DirectDraw* ddraw = Main::FindDirectDrawByWindow(hWnd);
		if (ddraw)
			ddraw->ScaleMouseIn(&lParam);

		mousePos = lParam;
		mousehWnd = hWnd;

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_NCMOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_NCMOUSELEAVE:
	{
		if (configDisplayWindowed && Main::FindDirectDrawByWindow(hWnd))
			while (ShowCursor(TRUE) <= 0);

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	default:
		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	return NULL;
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	DirectDraw* ddraw = (DirectDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hWnd);
	{
		if (!ddraw->wasPixelSet)
		{
			ddraw->wasPixelSet = TRUE;
			PIXELFORMATDESCRIPTOR pfd = { NULL };
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = configDisplayResolution.bpp;
			pfd.cAlphaBits = 0;
			pfd.cAccumBits = 0;
			pfd.cDepthBits = 0;
			pfd.cStencilBits = 0;
			pfd.cAuxBuffers = 0;
			pfd.iLayerType = PFD_MAIN_PLANE;

			DWORD pfIndex;
			if (!GL::PreparePixelFormat(&pfd, &pfIndex, ddraw->hWnd))
			{
				pfIndex = ChoosePixelFormat(ddraw->hDc, &pfd);
				if (pfIndex == NULL)
					Main::ShowError("ChoosePixelFormat failed", __FILE__, __LINE__);
				else if (pfd.dwFlags & PFD_NEED_PALETTE)
					Main::ShowError("Needs palette", __FILE__, __LINE__);
			}

			if (!SetPixelFormat(ddraw->hDc, pfIndex, &pfd))
				Main::ShowError("SetPixelFormat failed", __FILE__, __LINE__);

			memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			if (DescribePixelFormat(ddraw->hDc, pfIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
				Main::ShowError("DescribePixelFormat failed", __FILE__, __LINE__);

			if ((pfd.iPixelType != PFD_TYPE_RGBA) ||
				(pfd.cRedBits < 5) || (pfd.cGreenBits < 5) || (pfd.cBlueBits < 5))
				Main::ShowError("Bad pixel type", __FILE__, __LINE__);
		}

		HGLRC hRc = WGLCreateContext(ddraw->hDc);
		{
			WGLMakeCurrent(ddraw->hDc, hRc);
			{
				GL::CreateContextAttribs(ddraw->hDc, &hRc);

				GLint glMaxTexSize;
				GLGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexSize);

				if (glVersion >= GL_VER_3_0)
				{
					DWORD maxSize = ddraw->virtualMode->dwWidth > ddraw->virtualMode->dwHeight ? ddraw->virtualMode->dwWidth : ddraw->virtualMode->dwHeight;

					DWORD maxTexSize = 1;
					while (maxTexSize < maxSize)
						maxTexSize <<= 1;

					if (maxTexSize > glMaxTexSize)
						glVersion = GL_VER_1_4;
				}

				if (configGlVersion == GL_VER_3)
				{
					if (glVersion >= GL_VER_3_0)
						ddraw->RenderNew();
					else
						Main::ShowError("OpenGL 3.0 is not supported", __FILE__, __LINE__);
				}
				else if (configGlVersion == GL_VER_1)
					ddraw->RenderOld(glMaxTexSize);
				else if (glVersion >= GL_VER_3_0)
					ddraw->RenderNew();
				else
					ddraw->RenderOld(glMaxTexSize);
			}
			WGLMakeCurrent(ddraw->hDc, NULL);
		}
		WGLDeleteContext(hRc);
	}
	::ReleaseDC(ddraw->hWnd, ddraw->hDc);
	ddraw->hDc = NULL;

	return NULL;
}

VOID DirectDraw::RenderOld(DWORD glMaxTexSize)
{
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size;
	if (glCapsClampToEdge == GL_CLAMP_TO_EDGE)
		size = this->virtualMode->dwWidth < this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;
	else
		size = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;

	DWORD maxAllow = 1;
	while (maxAllow < size)
		maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;

	DWORD framePerWidth = this->virtualMode->dwWidth / maxTexSize + (this->virtualMode->dwWidth % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->virtualMode->dwHeight / maxTexSize + (this->virtualMode->dwHeight % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;

	Frame* frames = (Frame*)malloc(frameCount * sizeof(Frame));
	{
		Frame* frame = frames;
		for (DWORD y = 0; y < this->virtualMode->dwHeight; y += maxTexSize)
		{
			DWORD height = this->virtualMode->dwHeight - y;
			if (height > maxTexSize)
				height = maxTexSize;

			for (DWORD x = 0; x < this->virtualMode->dwWidth; x += maxTexSize, ++frame)
			{
				DWORD width = this->virtualMode->dwWidth - x;
				if (width > maxTexSize)
					width = maxTexSize;

				frame->rect.x = x;
				frame->rect.y = y;
				frame->rect.width = width;
				frame->rect.height = height;

				frame->vSize.width = x + width;
				frame->vSize.height = y + height;

				frame->tSize.width = width == maxTexSize ? 1.0 : (FLOAT)width / maxTexSize;
				frame->tSize.height = height == maxTexSize ? 1.0 : (FLOAT)height / maxTexSize;

				GLGenTextures(2, frame->id);
				for (DWORD s = 0; s < 2; ++s)
				{
					GLBindTexture(GL_TEXTURE_2D, frame->id[s]);

					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);

					GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

					if (this->virtualMode->dwBPP == 8)
					{
						if (GLColorTable)
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, maxTexSize, maxTexSize, GL_NONE, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, NULL);
						else
							GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					}
					else if (this->virtualMode->dwBPP == 16 && glVersion > GL_VER_1_1)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, maxTexSize, maxTexSize, GL_NONE, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				}
			}
		}

		// Draw
		GLClearColor(0.0, 0.0, 0.0, 1.0);

		GLMatrixMode(GL_PROJECTION);
		GLLoadIdentity();
		GLOrtho(0.0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, 0.0, 0.0, 1.0);
		GLMatrixMode(GL_MODELVIEW);
		GLLoadIdentity();

		GLEnable(GL_TEXTURE_2D);

		DWORD fpsQueue[FPS_COUNT];
		DWORD tickQueue[FPS_COUNT];

		DWORD fpsIdx = -1;
		DWORD fpsTotal = 0;
		DWORD fpsCount = 0;
		INT fpsSum = 0;

		if (configFpsCounter && this->virtualMode->dwBPP != 8)
		{
			memset(fpsQueue, 0, sizeof(fpsQueue));
			memset(tickQueue, 0, sizeof(tickQueue));
		}

		VOID* pixelBuffer = malloc(maxTexSize * maxTexSize * sizeof(DWORD));
		{
			do
			{
				DirectDrawSurface* surface = (DirectDrawSurface*)this->attachedSurface;
				if (surface)
				{
					if (configFpsCounter && this->virtualMode->dwBPP == 16)
					{
						DWORD tick = GetTickCount();

						if (isFpsChanged)
						{
							isFpsChanged = FALSE;
							fpsIdx = -1;
							fpsTotal = 0;
							fpsCount = 0;
							fpsSum = 0;
							memset(fpsQueue, 0, sizeof(fpsQueue));
							memset(tickQueue, 0, sizeof(tickQueue));
						}

						++fpsTotal;
						if (fpsCount < FPS_COUNT)
							++fpsCount;

						++fpsIdx;
						if (fpsIdx == FPS_COUNT)
							fpsIdx = 0;

						DWORD diff = tick - tickQueue[fpsTotal != fpsCount ? fpsIdx : 0];
						tickQueue[fpsIdx] = tick;

						DWORD fps = diff ? Main::Round(1000.0 / diff * fpsCount) : 9999;

						DWORD* queue = &fpsQueue[fpsIdx];
						fpsSum -= *queue - fps;
						*queue = fps;
					}

					this->CheckView();

					BOOL updateFilter = this->isStateChanged;
					if (this->isStateChanged)
						this->isStateChanged = FALSE;

					DWORD count = frameCount;
					Frame* frame = frames;
					while (count--)
					{
						GLBindTexture(GL_TEXTURE_2D, frame->id[surface->index]);

						if (updateFilter)
						{
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
							GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
						}

						if (this->virtualMode->dwBPP == 8)
						{
							if (GLColorTable)
							{
								GLColorTable(GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, surface->attachedPallete->entries);

								BYTE* pix = (BYTE*)pixelBuffer;
								for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
								{
									BYTE* idx = surface->indexBuffer + y * this->virtualMode->dwWidth + frame->rect.x;
									memcpy(pix, idx, frame->rect.width);
									pix += frame->rect.width;
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, pixelBuffer);
							}
							else
							{
								PALETTEENTRY* pix = (PALETTEENTRY*)pixelBuffer;
								for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
								{
									BYTE* idx = surface->indexBuffer + y * this->virtualMode->dwWidth + frame->rect.x;
									for (DWORD x = frame->rect.x; x < frame->vSize.width; ++x)
										*pix++ = surface->attachedPallete->entries[*idx++];
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
							}
						}
						else if (this->virtualMode->dwBPP == 16)
						{
							if (glVersion > GL_VER_1_1)
							{
								if (frameCount > 1)
								{
									WORD* pix = (WORD*)pixelBuffer;
									for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
									{
										WORD* idx = (WORD*)surface->indexBuffer + y * this->virtualMode->dwWidth + frame->rect.x;
										memcpy(pix, idx, frame->rect.width * sizeof(WORD));
										pix += frame->rect.width;
									}
								}
								else
									memcpy(pixelBuffer, surface->indexBuffer, frame->rect.width * frame->rect.height * sizeof(WORD));

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, pixelBuffer);

								if (configFpsCounter && count == frameCount - 1)
								{
									DWORD fps = Main::Round((FLOAT)fpsSum / fpsCount);

									DWORD offset = FPS_X;

									DWORD digCount = 0;
									DWORD current = fps;
									do
									{
										++digCount;
										current = current / 10;
									} while (current);

									DWORD dcount = digCount;
									current = fps;
									do
									{
										DWORD digit = current % 10;
										bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * digit;

										for (DWORD y = 0; y < FPS_HEIGHT; ++y)
										{
											WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->virtualMode->dwWidth +
												FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

											WORD* pix = (WORD*)pixelBuffer + y * (FPS_STEP + FPS_WIDTH) * digCount +
												(FPS_STEP + FPS_WIDTH) * (dcount - 1);

											memcpy(pix, idx, FPS_STEP * sizeof(WORD));
											pix += FPS_STEP;
											idx += FPS_STEP;

											DWORD width = FPS_WIDTH;
											do
											{
												*pix++ = *lpDig++ ? 0xFFFF : *idx;
												++idx;
											} while (--width);
										}

										current = current / 10;
									} while (--dcount);

									GLPixelStorei(GL_UNPACK_ALIGNMENT, 2);
									GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_WIDTH + FPS_STEP) * digCount, FPS_HEIGHT, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, pixelBuffer);
									GLPixelStorei(GL_UNPACK_ALIGNMENT, 4);
								}
							}
							else
							{
								DWORD* pix = (DWORD*)pixelBuffer;
								for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
								{
									WORD* idx = (WORD*)surface->indexBuffer + y * this->virtualMode->dwWidth + frame->rect.x;
									for (DWORD x = frame->rect.x; x < frame->vSize.width; ++x)
									{
										WORD px = *idx++;
										*pix++ = ((px & 0x7C00) >> 7) | ((px & 0x3E0) << 6) | ((px & 0x1F) << 19);
									}
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);

								if (configFpsCounter && count == frameCount - 1)
								{
									DWORD fps = Main::Round((FLOAT)fpsSum / fpsCount);

									DWORD offset = FPS_X;

									DWORD digCount = 0;
									DWORD current = fps;
									do
									{
										++digCount;
										current = current / 10;
									} while (current);

									DWORD dcount = digCount;
									current = fps;
									do
									{
										DWORD digit = current % 10;
										bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * digit;

										for (DWORD y = 0; y < FPS_HEIGHT; ++y)
										{
											WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->virtualMode->dwWidth +
												FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

											DWORD* pix = (DWORD*)pixelBuffer + y * (FPS_STEP + FPS_WIDTH) * digCount +
												(FPS_STEP + FPS_WIDTH) * (dcount - 1);

											for (DWORD x = 0; x < FPS_STEP; ++x)
											{
												WORD px = *idx++;
												*pix++ = ((px & 0x7C00) >> 7) | ((px & 0x3E0) << 6) | ((px & 0x1F) << 19);
											}

											DWORD width = FPS_WIDTH;
											do
											{
												WORD px = *idx++;
												*pix++ = *lpDig++ ? 0xFFFFFFFF : (((px & 0x7C00) >> 7) | ((px & 0x3E0) << 6) | ((px & 0x1F) << 19));
											} while (--width);
										}

										current = current / 10;
									} while (--dcount);

									GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_WIDTH + FPS_STEP) * digCount, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
								}
							}
						}
						else
						{
							DWORD* pix = (DWORD*)pixelBuffer;
							for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
							{
								DWORD* idx = (DWORD*)surface->indexBuffer + y * this->virtualMode->dwWidth + frame->rect.x;
								memcpy(pix, idx, frame->rect.width * sizeof(DWORD));
								pix += frame->rect.width;
							}

							GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);

							/*if (configFpsCounter && count == frameCount - 1)
							{
								DWORD fps = Main::Round((FLOAT)fpsSum / fpsCount);

								DWORD offset = FPS_X;

								DWORD digCount = 0;
								DWORD current = fps;
								do
								{
									++digCount;
									current = current / 10;
								} while (current);

								DWORD dcount = digCount;
								current = fps;
								do
								{
									DWORD digit = current % 10;
									bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * digit;

									for (DWORD y = 0; y < FPS_HEIGHT; ++y)
									{
										DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->virtualMode->dwWidth +
											FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

										DWORD* pix = (DWORD*)pixelBuffer + y * (FPS_STEP + FPS_WIDTH) * digCount +
											(FPS_STEP + FPS_WIDTH) * (dcount - 1);

										memcpy(pix, idx, FPS_STEP * sizeof(DWORD));
										pix += FPS_STEP;
										idx += FPS_STEP;

										DWORD width = FPS_WIDTH;
										do
										{
											*pix++ = *lpDig++ ? 0xFFFFFFFF : *idx;
											++idx;
										} while (--width);
									}

									current = current / 10;
								} while (--dcount);

								GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_WIDTH + FPS_STEP) * digCount, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
							}*/
						}

						GLBegin(GL_TRIANGLE_FAN);
						{
							GLTexCoord2f(0.0, 0.0);
							GLVertex2s(frame->rect.x, frame->rect.y);

							GLTexCoord2f(frame->tSize.width, 0.0);
							GLVertex2s(frame->vSize.width, frame->rect.y);

							GLTexCoord2f(frame->tSize.width, frame->tSize.height);
							GLVertex2s(frame->vSize.width, frame->vSize.height);

							GLTexCoord2f(0.0, frame->tSize.height);
							GLVertex2s(frame->rect.x, frame->vSize.height);
						}
						GLEnd();
						++frame;
					}

					GLFlush();
					SwapBuffers(this->hDc);

					WaitForSingleObject(this->hDrawEvent, INFINITE);
					ResetEvent(this->hDrawEvent);
				}
			} while (!this->isFinish);
			GLFinish();
		}
		free(pixelBuffer);

		// Remove
		frame = frames;
		DWORD count = frameCount;
		while (count--)
		{
			GLDeleteTextures(2, frame->id);
			++frame;
		}
	}
	free(frames);
}

VOID DirectDraw::RenderNew()
{
	DWORD maxSize = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize) maxTexSize <<= 1;
	FLOAT texWidth = this->virtualMode->dwWidth == maxTexSize ? 1.0 : (FLOAT)this->virtualMode->dwWidth / maxTexSize;
	FLOAT texHeight = this->virtualMode->dwHeight == maxTexSize ? 1.0 : (FLOAT)this->virtualMode->dwHeight / maxTexSize;

	FLOAT buffer[4][4] = {
		{ 0.0, 0.0, 0.0, 0.0 },
		{ this->virtualMode->dwWidth, 0.0, texWidth, 0.0 },
		{ this->virtualMode->dwWidth, this->virtualMode->dwHeight, texWidth, texHeight },
		{ 0.0, this->virtualMode->dwHeight, 0.0, texHeight }
	};

	FLOAT mvpMatrix[4][4] = {
		{ 2.0f / this->virtualMode->dwWidth, 0.0f, 0.0f, 0.0f },
		{ 0.0f, -2.0f / this->virtualMode->dwHeight, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	GLuint vShader, fShaderWin, fShaderFull;
	GLuint shProgramWin, shProgramFull;
	GLuint textures[3];
	GLuint arrayName, bufferName;


	shProgramWin = GLCreateProgram();
	{
		vShader = GL::CompileShaderSource(shVertexSoure, GL_VERTEX_SHADER, this->hWnd);
		fShaderWin = GL::CompileShaderSource(shFragmentWindowed, GL_FRAGMENT_SHADER, this->hWnd);

		GLAttachShader(shProgramWin, vShader);
		GLAttachShader(shProgramWin, fShaderWin);

		GLLinkProgram(shProgramWin);
		GLUseProgram(shProgramWin);
		{
			GLUniformMatrix4fv(GLGetUniformLocation(shProgramWin, "mvp"), 1, GL_FALSE, (GLfloat*)mvpMatrix);
			GLUniform1i(GLGetUniformLocation(shProgramWin, "tex01"), 0);

			DWORD texCount = this->virtualMode->dwBPP == 8 ? 3 : 1;
			GLGenTextures(texCount, textures);
			{
				if (this->virtualMode->dwBPP == 8)
				{
					fShaderFull = GL::CompileShaderSource(shFragmentFullscreen, GL_FRAGMENT_SHADER, this->hWnd);
					shProgramFull = GLCreateProgram();
					{
						GLAttachShader(shProgramFull, vShader);
						GLAttachShader(shProgramFull, fShaderFull);

						GLLinkProgram(shProgramFull);
						GLUseProgram(shProgramFull);
						{
							GLUniformMatrix4fv(GLGetUniformLocation(shProgramFull, "mvp"), 1, GL_FALSE, (GLfloat*)mvpMatrix);
							GLUniform1i(GLGetUniformLocation(shProgramFull, "tex01"), 0);
							GLUniform1i(GLGetUniformLocation(shProgramFull, "pal01"), 1);
						}
					}

					GLuint paletteId = textures[0];
					GLuint indicesId = textures[1];
					GLuint textureId = textures[2];

					GLActiveTexture(GL_TEXTURE1);
					GLBindTexture(GL_TEXTURE_1D, paletteId);

					GLTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					GLTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					GLTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

					GLActiveTexture(GL_TEXTURE0);
					GLBindTexture(GL_TEXTURE_2D, indicesId);

					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_R8, maxTexSize, maxTexSize, GL_NONE, GL_RED, GL_UNSIGNED_BYTE, NULL);

					GLBindTexture(GL_TEXTURE_2D, textureId);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

				}
				else
				{
					GLActiveTexture(GL_TEXTURE0);
					GLBindTexture(GL_TEXTURE_2D, textures[0]);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
					GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);

					if (this->virtualMode->dwBPP == 16)
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, maxTexSize, maxTexSize, GL_NONE, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
					else
						GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxTexSize, maxTexSize, GL_NONE, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
				}

				GLGenVertexArrays(1, &arrayName);
				{
					GLBindVertexArray(arrayName);
					{
						GLGenBuffers(1, &bufferName);
						{
							GLBindBuffer(GL_ARRAY_BUFFER, bufferName);
							{
								GLBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STREAM_DRAW);

								GLint attrCoordsLoc = GLGetAttribLocation(shProgramWin, "vCoord");
								GLEnableVertexAttribArray(attrCoordsLoc);
								GLVertexAttribPointer(attrCoordsLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)0);

								GLint attrTexCoordsLoc = GLGetAttribLocation(shProgramWin, "vTexCoord");
								GLEnableVertexAttribArray(attrTexCoordsLoc);
								GLVertexAttribPointer(attrTexCoordsLoc, 2, GL_FLOAT, GL_FALSE, 16, (GLvoid*)8);
								{
									GLClearColor(0.0, 0.0, 0.0, 1.0);

									DWORD fpsQueue[FPS_COUNT];
									DWORD tickQueue[FPS_COUNT];

									DWORD fpsIdx = -1;
									DWORD fpsTotal = 0;
									DWORD fpsCount = 0;
									INT fpsSum = 0;

									if (configFpsCounter && this->virtualMode->dwBPP != 8)
									{
										memset(fpsQueue, 0, sizeof(fpsQueue));
										memset(tickQueue, 0, sizeof(tickQueue));
									}

									VOID* pixelBuffer = malloc(maxTexSize * maxTexSize * sizeof(DWORD));
									{
										do
										{
											DirectDrawSurface* surface = (DirectDrawSurface*)this->attachedSurface;
											if (surface)
											{
												if (configFpsCounter && this->virtualMode->dwBPP == 16)
												{
													DWORD tick = GetTickCount();

													if (isFpsChanged)
													{
														isFpsChanged = FALSE;
														fpsIdx = -1;
														fpsTotal = 0;
														fpsCount = 0;
														fpsSum = 0;
														memset(fpsQueue, 0, sizeof(fpsQueue));
														memset(tickQueue, 0, sizeof(tickQueue));
													}

													++fpsTotal;
													if (fpsCount < FPS_COUNT)
														++fpsCount;

													++fpsIdx;
													if (fpsIdx == FPS_COUNT)
														fpsIdx = 0;

													DWORD diff = tick - tickQueue[fpsTotal != fpsCount ? fpsIdx : 0];
													tickQueue[fpsIdx] = tick;

													DWORD fps = diff ? Main::Round(1000.0 / diff * fpsCount) : 9999;

													DWORD* queue = &fpsQueue[fpsIdx];
													fpsSum -= *queue - fps;
													*queue = fps;
												}

												this->CheckView();

												if (this->virtualMode->dwBPP == 8)
												{
													if (configGlFiltering == GL_LINEAR)
													{
														if (this->isStateChanged)
														{
															this->isStateChanged = FALSE;
															GLUseProgram(shProgramWin);

															GLuint textureId = textures[2];
															GLActiveTexture(GL_TEXTURE0);
															GLBindTexture(GL_TEXTURE_2D, textureId);
														}

														BYTE* idx = surface->indexBuffer;
														PALETTEENTRY* pix = (PALETTEENTRY*)pixelBuffer;
														DWORD count = this->virtualMode->dwWidth * this->virtualMode->dwHeight;
														while (count--) *pix++ = surface->attachedPallete->entries[*idx++];

														GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
													}
													else
													{
														GLuint paletteId = textures[0];
														GLuint indicesId = textures[1];

														if (this->isStateChanged)
														{
															this->isStateChanged = FALSE;
															GLUseProgram(shProgramFull);
														}

														GLActiveTexture(GL_TEXTURE1);
														GLBindTexture(GL_TEXTURE_1D, paletteId);
														GLTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA, GL_UNSIGNED_BYTE, surface->attachedPallete->entries);

														GLActiveTexture(GL_TEXTURE0);
														GLBindTexture(GL_TEXTURE_2D, indicesId);
														GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_RED, GL_UNSIGNED_BYTE, surface->indexBuffer);
													}
												}
												else if (this->virtualMode->dwBPP == 16)
												{
													if (this->isStateChanged)
													{
														this->isStateChanged = FALSE;

														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
													}

													GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, surface->indexBuffer);

													if (configFpsCounter)
													{
														DWORD fps = Main::Round((FLOAT)fpsSum / fpsCount);

														DWORD offset = FPS_X;

														DWORD digCount = 0;
														DWORD current = fps;
														do
														{
															++digCount;
															current = current / 10;
														} while (current);

														DWORD dcount = digCount;
														current = fps;
														do
														{
															DWORD digit = current % 10;
															bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * digit;

															for (DWORD y = 0; y < FPS_HEIGHT; ++y)
															{
																WORD* idx = (WORD*)surface->indexBuffer + (FPS_Y + y) * this->virtualMode->dwWidth +
																	FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

																WORD* pix = (WORD*)pixelBuffer + y * (FPS_STEP + FPS_WIDTH) * digCount +
																	(FPS_STEP + FPS_WIDTH) * (dcount - 1);

																memcpy(pix, idx, FPS_STEP * sizeof(WORD));
																pix += FPS_STEP;;
																idx += FPS_STEP;

																DWORD width = FPS_WIDTH;
																do
																{
																	*pix++ = *lpDig++ ? 0xFFFF : *idx;
																	++idx;
																} while (--width);
															}

															current = current / 10;
														} while (--dcount);

														GLPixelStorei(GL_UNPACK_ALIGNMENT, 2);
														GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_WIDTH + FPS_STEP) * digCount, FPS_HEIGHT, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, pixelBuffer);
														GLPixelStorei(GL_UNPACK_ALIGNMENT, 4);
													}
												}
												else
												{
													if (this->isStateChanged)
													{
														this->isStateChanged = FALSE;

														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
														GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
													}

													GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_RGBA, GL_UNSIGNED_BYTE, surface->indexBuffer);

													/*if (configFpsCounter)
													{
														DWORD fps = Main::Round((FLOAT)fpsSum / fpsCount);

														DWORD offset = FPS_X;

														DWORD digCount = 0;
														DWORD current = fps;
														do
														{
															++digCount;
															current = current / 10;
														} while (current);

														DWORD dcount = digCount;
														current = fps;
														do
														{
															DWORD digit = current % 10;
															bool* lpDig = (bool*)counters + FPS_WIDTH * FPS_HEIGHT * digit;

															for (DWORD y = 0; y < FPS_HEIGHT; ++y)
															{
																DWORD* idx = (DWORD*)surface->indexBuffer + (FPS_Y + y) * this->virtualMode->dwWidth +
																	FPS_X + (FPS_STEP + FPS_WIDTH) * (dcount - 1);

																DWORD* pix = (DWORD*)pixelBuffer + y * (FPS_STEP + FPS_WIDTH) * digCount +
																	(FPS_STEP + FPS_WIDTH) * (dcount - 1);

																memcpy(pix, idx, FPS_STEP * sizeof(DWORD));
																pix += FPS_STEP;;
																idx += FPS_STEP;

																DWORD width = FPS_WIDTH;
																do
																{
																	*pix++ = *lpDig++ ? 0xFFFFFFFF : *idx;
																	++idx;
																} while (--width);
															}

															current = current / 10;
														} while (--dcount);

														GLTexSubImage2D(GL_TEXTURE_2D, 0, FPS_X, FPS_Y, (FPS_WIDTH + FPS_STEP) * digCount, FPS_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
													}*/
												}

												GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

												GLFlush();
												SwapBuffers(this->hDc);

												WaitForSingleObject(this->hDrawEvent, INFINITE);
												ResetEvent(this->hDrawEvent);
											}
										} while (!this->isFinish);
										GLFinish();
									}
									free(pixelBuffer);
								}
							}
							GLBindBuffer(GL_ARRAY_BUFFER, NULL);
						}
						GLDeleteBuffers(1, &bufferName);
					}
					GLBindVertexArray(NULL);
				}
				GLDeleteVertexArrays(1, &arrayName);
			}
			GLDeleteTextures(texCount, textures);
		}
		GLUseProgram(NULL);

		if (this->virtualMode->dwBPP == 8)
			GLDeleteProgram(shProgramFull);
	}
	GLDeleteProgram(shProgramWin);
}


DirectDraw::DirectDraw(DirectDraw* lastObj)
{
	GL::Load();

	this->last = lastObj;
	this->isStylesLoaded = NULL;
	this->realMode = NULL;
	this->virtualMode = NULL;
	this->attachedSurface = NULL;
	this->tick = GetTickCount();

	this->hDrawEvent = CreateEvent(0, TRUE, FALSE, 0);
}

DirectDraw::~DirectDraw()
{
	DirectDrawSurface* surfaceEntry = (DirectDrawSurface*)this->surfaceEntries;
	while (surfaceEntry)
	{
		DirectDrawSurface* curr = surfaceEntry->prev;
		delete surfaceEntry;
		surfaceEntry = curr;
	}

	DirectDrawPalette* paletteEntry = (DirectDrawPalette*)this->paletteEntries;
	while (paletteEntry)
	{
		DirectDrawPalette* curr = paletteEntry->prev;
		delete paletteEntry;
		paletteEntry = curr;
	}

	DirectDrawClipper* clipperEntry = (DirectDrawClipper*)this->clipperEntries;
	while (clipperEntry)
	{
		DirectDrawClipper* curr = clipperEntry->prev;
		delete clipperEntry;
		clipperEntry = curr;
	}

	CloseHandle(this->hDrawEvent);
}

VOID DirectDraw::CalcView()
{
	this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
	this->viewport.point.x = this->viewport.point.y = 0.0f;

	this->viewport.rectangle.width = this->viewport.width;
	this->viewport.rectangle.height = this->viewport.height;

	this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / this->virtualMode->dwWidth;
	this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / this->virtualMode->dwHeight;

	if (configDisplayAspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
	{
		if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
		{
			FLOAT fw = this->viewport.viewFactor.y * this->virtualMode->dwWidth;
			this->viewport.rectangle.width = Main::Round(fw);

			this->viewport.point.x = ((FLOAT)this->viewport.width - fw) / 2.0f;
			this->viewport.rectangle.x = Main::Round(this->viewport.point.x);

			this->viewport.clipFactor.x = this->viewport.viewFactor.y;
		}
		else
		{
			FLOAT fh = this->viewport.viewFactor.x * this->virtualMode->dwHeight;
			this->viewport.rectangle.height = Main::Round(fh);

			this->viewport.point.y = ((FLOAT)this->viewport.height - fh) / 2.0f;
			this->viewport.rectangle.y = Main::Round(this->viewport.point.y);

			this->viewport.clipFactor.y = this->viewport.viewFactor.x;
		}
	}
}

VOID DirectDraw::CheckView()
{
	if (this->viewport.refresh)
	{
		this->CalcView();
		GLViewport(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height);

		this->clearStage = 0;
	}

	if (++this->clearStage <= 2)
		GLClear(GL_COLOR_BUFFER_BIT);
}

HRESULT DirectDraw::SetFullscreenMode()
{
	DEVMODE devMode = { NULL };
	devMode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode);

	devMode.dmPelsWidth = this->realMode->dwWidth;
	devMode.dmPelsHeight = this->realMode->dwHeight;
	if (configDisplayResolution.index > 1)
		devMode.dmBitsPerPel = configDisplayResolution.bpp;
	devMode.dmDisplayFrequency = this->realMode->dwFrequency;
	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	DWORD res = ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_FULLSCREEN | CDS_TEST | CDS_RESET, NULL);
	if (res != DISP_CHANGE_SUCCESSFUL)
		return DDERR_INVALIDMODE;

	ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_FULLSCREEN | CDS_RESET, NULL);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);

	RECT rect = { 0, -1, this->realMode->dwWidth, this->realMode->dwHeight };
	AdjustWindowRect(&rect, FS_STYLE, FALSE);

	rect.left += devMode.dmPosition.x;
	rect.right += devMode.dmPosition.x;
	rect.top += devMode.dmPosition.y;
	rect.bottom += devMode.dmPosition.y;
	SetWindowLong(this->hWnd, GWL_STYLE, FS_STYLE);
	SetWindowPos(this->hWnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, NULL);
	SetForegroundWindow(this->hWnd);

	while (ShowCursor(FALSE) >= -1);

	return DD_OK;
}

HRESULT DirectDraw::SetWindowedMode()
{
	if (!this->isStylesLoaded)
	{
		this->isStylesLoaded = TRUE;
		MONITORINFO mi = { sizeof(mi) };
		HMONITOR hMon = MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hMon, &mi);
		DWORD monWidth = mi.rcWork.right - mi.rcWork.left;
		DWORD monHeight = mi.rcWork.bottom - mi.rcWork.top;
		DWORD newWidth = 0.75 * (mi.rcWork.right - mi.rcWork.left);
		DWORD newHeght = 0.75 * (mi.rcWork.bottom - mi.rcWork.top);

		FLOAT k = (FLOAT)this->virtualMode->dwWidth / this->virtualMode->dwHeight;

		DWORD check = (FLOAT)newHeght * k;
		if (newWidth > check)
			newWidth = check;
		else
			newHeght = (FLOAT)newWidth / k;

		RECT* rect = &this->windowPlacement.rcNormalPosition;
		rect->left = mi.rcWork.left + (monWidth - newWidth) / 2;
		rect->top = mi.rcWork.top + (monHeight - newHeght) / 2;
		rect->right = rect->left + newWidth;
		rect->bottom = rect->top + newHeght;
		AdjustWindowRect(rect, WIN_STYLE, FALSE);

		this->windowPlacement.ptMinPosition.x = this->windowPlacement.ptMinPosition.y = -1;
		this->windowPlacement.ptMaxPosition.x = this->windowPlacement.ptMaxPosition.y = -1;

		this->windowPlacement.flags = NULL;
		this->windowPlacement.showCmd = SW_SHOWNORMAL;
	}

	DWORD check = GetWindowLong(this->hWnd, GWL_STYLE) & WS_SIZEBOX;
	if (!check)
	{
		SetWindowLong(this->hWnd, GWL_STYLE, WIN_STYLE);
		SetWindowLong(this->hWnd, GWL_EXSTYLE, WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
		SetWindowPlacement(this->hWnd, &this->windowPlacement);
	}

	return DD_OK;
}

VOID DirectDraw::ScaleMouseIn(LPARAM* lParam)
{
	if (configDisplayWindowed || this->virtualMode != this->realMode)
	{
		INT xPos = GET_X_LPARAM(*lParam);
		INT yPos = GET_Y_LPARAM(*lParam);

		if (xPos < this->viewport.rectangle.x)
			xPos = 0;
		else if (xPos >= this->viewport.rectangle.x + this->viewport.rectangle.width)
			xPos = this->virtualMode->dwWidth - 1;
		else
		{
			FLOAT number = (FLOAT)(xPos - this->viewport.rectangle.x) / this->viewport.clipFactor.x;
			FLOAT floorVal = floor(number);
			xPos = INT(floorVal + 0.5f > number ? floorVal : ceil(number));
		}

		if (yPos < this->viewport.rectangle.y)
			yPos = 0;
		else if (yPos >= this->viewport.rectangle.y + this->viewport.rectangle.height)
			yPos = this->virtualMode->dwHeight - 1;
		else
		{
			FLOAT number = (FLOAT)(yPos - this->viewport.rectangle.y) / this->viewport.clipFactor.y;
			FLOAT floorVal = floor(number);
			yPos = INT(floorVal + 0.5f > number ? floorVal : ceil(number));
		}

		*lParam = MAKELONG(xPos, yPos);
	}
}

VOID DirectDraw::ScaleMouseOut(LPARAM* lParam)
{
	if (configDisplayWindowed || this->virtualMode != this->realMode)
	{
		INT xPos = GET_X_LPARAM(*lParam);
		{
			FLOAT number = this->viewport.clipFactor.x * xPos;
			FLOAT floorVal = floor(number);
			xPos = this->viewport.rectangle.x + INT(floorVal + 0.5f > number ? floorVal : ceil(number));
		}

		if (xPos < this->viewport.rectangle.x)
			xPos = 0;
		else if (xPos >= this->viewport.rectangle.x + this->viewport.rectangle.width)
			xPos = this->viewport.rectangle.x + this->viewport.rectangle.width;

		INT yPos = GET_Y_LPARAM(*lParam);
		{
			FLOAT number = this->viewport.clipFactor.y * yPos;
			FLOAT floorVal = floor(number);
			yPos = this->viewport.rectangle.y + INT(floorVal + 0.5f > number ? floorVal : ceil(number));
		}

		if (yPos < this->viewport.rectangle.y)
			yPos = 0;
		else if (yPos >= this->viewport.rectangle.y + this->viewport.rectangle.height)
			yPos = this->viewport.rectangle.y + this->viewport.rectangle.height;

		*lParam = MAKELONG(xPos, yPos);
	}
}

VOID DirectDraw::CheckDisplayMode()
{
	DisplayMode* mode = this->virtualMode;
	if (configDisplayResolution.index)
		this->realMode = &modesList[6 + (mode->dwBPP == 32 ? 2 : (mode->dwBPP == 16 ? 1 : 0))];
	else
	{
		this->realMode = NULL;
		do
		{
			if (mode->dwExists)
				this->realMode = mode;

			mode += 3;
		} while (!this->realMode);
	}
}

// CALLED
ULONG DirectDraw::Release()
{
	if (ddrawList == this)
		ddrawList = NULL;
	else
	{
		DirectDraw* ddraw = ddrawList;
		while (ddraw)
		{
			if (ddraw->last == this)
			{
				ddraw->last = this->last;
				break;
			}

			ddraw = ddraw->last;
		}
	}

	delete this;
	return 0;
}

HRESULT DirectDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	this->clipperEntries = (LPDIRECTDRAWCLIPPER)new DirectDrawClipper(this);
	*lplpDDClipper = this->clipperEntries;

	return DD_OK;
}

HRESULT DirectDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE* lplpDDPalette, IUnknown* pUnkOuter)
{
	this->paletteEntries = (LPDIRECTDRAWPALETTE)new DirectDrawPalette(this);
	*lplpDDPalette = this->paletteEntries;

	this->paletteEntries->SetEntries(0, 0, 256, lpDDColorArray);

	return DD_OK;
}

HRESULT DirectDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	DirectDrawSurface* surface = new DirectDrawSurface(this, TRUE);
	*lplpDDSurface = (LPDIRECTDRAWSURFACE)surface;

	if (this->virtualMode)
	{
		lpDDSurfaceDesc->dwWidth = this->virtualMode->dwWidth;
		lpDDSurfaceDesc->dwHeight = this->virtualMode->dwHeight;
		lpDDSurfaceDesc->lPitch = this->virtualMode->dwWidth * (this->virtualMode->dwBPP >> 3);
	}

	this->isFinish = FALSE;
	this->viewport.refresh = TRUE;
	this->isStateChanged = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs = { NULL };
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, BELOW_NORMAL_PRIORITY_CLASS, &threadId);

	return DD_OK;
}

HRESULT DirectDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
	DisplayMode* mode = modesList;
	DWORD i;
	for (i = 0; i < 2; ++i)
	{
		for (DWORD j = 0; j < 3; ++j, ++mode)
		{
			mode->dwWidth = 320 << i;
			mode->dwHeight = 240 << i;
			mode->dwBPP = 8 << j;
			mode->dwExists = FALSE;
		}
	}

	DEVMODE devMode = { NULL };
	devMode.dmSize = sizeof(DEVMODE);

	DWORD bppCheck = 32;
	if (configDisplayResolution.index > 1)
		bppCheck = configDisplayResolution.bpp;
	else if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
	{
		bppCheck = devMode.dmBitsPerPel;
		memset(&devMode, NULL, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
	}

	for (i = 0; EnumDisplaySettings(NULL, i, &devMode); ++i)
	{
		INT idx = -1;
		if (devMode.dmBitsPerPel == bppCheck)
		{
			if (devMode.dmPelsWidth == 320 && devMode.dmPelsHeight == 240)
				idx = 0;
			else if (devMode.dmPelsWidth == 640 && devMode.dmPelsHeight == 480)
				idx = 3;
		}

		if (idx >= 0)
		{
			mode = &modesList[idx];
			for (DWORD j = 0; j < 3; ++j, ++mode)
			{
				if (mode->dwFrequency < devMode.dmDisplayFrequency)
					mode->dwFrequency = devMode.dmDisplayFrequency;
			}
		}

		memset(&devMode, NULL, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
	}

	memset(&devMode, NULL, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
	{
		mode = &modesList[6];
		for (i = 0; i < 3; ++i, ++mode)
		{
			if (configDisplayResolution.index <= 1)
			{
				mode->dwWidth = devMode.dmPelsWidth;
				mode->dwHeight = devMode.dmPelsHeight;
			}
			else
			{
				mode->dwWidth = configDisplayResolution.width;
				mode->dwHeight = configDisplayResolution.height;
			}

			mode->dwFrequency = devMode.dmDisplayFrequency;
			mode->dwBPP = 8 << i;
		}
	}

	DDSURFACEDESC ddSurfaceDesc = { NULL };
	for (i = 0; i < 2; ++i)
	{
		ddSurfaceDesc.dwWidth = 320 << i;
		ddSurfaceDesc.dwHeight = 240 << i;

		BOOL isBreak = FALSE;
		for (DWORD j = 0; j < 3; ++j)
		{
			ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = 8 << j;
			if (!lpEnumModesCallback(&ddSurfaceDesc, NULL))
			{
				isBreak = TRUE;
				break;
			}
		}
		if (isBreak)
			break;
	}

	return DD_OK;
}

HRESULT DirectDraw::FlipToGDISurface() { return DD_OK; }

HRESULT DirectDraw::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	lpDDDriverCaps->dwCaps = DDCAPS_3D;
	return DD_OK;
}

HRESULT DirectDraw::RestoreDisplayMode()
{
	ChangeDisplaySettings(NULL, NULL);
	return DD_OK;
}

HRESULT DirectDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	if (hWnd && hWnd != this->hWnd)
	{
		this->hWnd = hWnd;
		this->windowPlacement.length = sizeof(WINDOWPLACEMENT);
		this->mbPressed = NULL;
		this->wasPixelSet = FALSE;

		if (!OldWindowProc)
			OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);

		this->hDc = NULL;
	}

	return DD_OK;
}

HRESULT DirectDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	DWORD idx;
	if (dwWidth == 320 && dwHeight == 240)
		idx = 0;
	else if (dwWidth == 640 && dwHeight == 480)
		idx = 3;
	else
		idx = 6;

	idx += dwBPP == 32 ? 2 : (dwBPP == 16 ? 1 : 0);

	this->virtualMode = &modesList[idx];
	this->CheckDisplayMode();

	if (configDisplayWindowed)
	{
		if (this->isStylesLoaded)
			GetWindowPlacement(hWnd, &this->windowPlacement);
		this->SetWindowedMode();
	}
	else
		this->SetFullscreenMode();

	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	return DD_OK;
}

HRESULT DirectDraw::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent) { return DD_OK; }