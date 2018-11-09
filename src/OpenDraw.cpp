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
#include "OpenDraw.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Main.h"
#include "CommCtrl.h"
#include "TextRenderer.h"

#pragma region Not Implemented
ULONG OpenDraw::AddRef() { return 0; }
HRESULT OpenDraw::Compact() { return DD_OK; }
HRESULT OpenDraw::EnumSurfaces(DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMSURFACESCALLBACK) { return DD_OK; }
HRESULT OpenDraw::GetDisplayMode(LPDDSURFACEDESC) { return DD_OK; }
HRESULT OpenDraw::GetFourCCCodes(LPDWORD, LPDWORD) { return DD_OK; }
HRESULT OpenDraw::GetGDISurface(LPDIRECTDRAWSURFACE *) { return DD_OK; }
HRESULT OpenDraw::GetMonitorFrequency(LPDWORD) { return DD_OK; }
HRESULT OpenDraw::GetScanLine(LPDWORD) { return DD_OK; }
HRESULT OpenDraw::GetVerticalBlankStatus(LPBOOL) { return DD_OK; }
HRESULT OpenDraw::Initialize(GUID *) { return DD_OK; }
HRESULT OpenDraw::DuplicateSurface(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE *) { return DD_OK; }
HRESULT OpenDraw::QueryInterface(REFIID riid, LPVOID* ppvObj) { return DD_OK; }
HRESULT OpenDraw::FlipToGDISurface() { return DD_OK; }
HRESULT OpenDraw::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent) { return DD_OK; }
#pragma endregion

#define WIN_STYLE WS_POPUPWINDOW | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE | WS_CLIPSIBLINGS | WS_SIZEBOX
#define FS_STYLE WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_CLIPSIBLINGS

DisplayMode modesList[12];

WNDPROC OldWindowProc, OldPanelProc;
LPARAM mousePos;
HWND mousehWnd;
HBRUSH blackBrush;

LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw && configDisplayWindowed)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_ERASEBKGND:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw && !configDisplayWindowed)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, blackBrush);

			return TRUE;
		}
		return NULL;
	}

	case WM_MOVE:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw && ddraw->hDraw)
		{
			DWORD stye = GetWindowLong(ddraw->hDraw, GWL_STYLE);
			if (stye & WS_POPUP)
			{
				POINT point = { LOWORD(lParam), HIWORD(lParam) };
				ScreenToClient(hWnd, &point);

				RECT rect;
				rect.left = point.x - LOWORD(lParam);
				rect.top = point.y - HIWORD(lParam);
				rect.right = rect.left + 256;
				rect.bottom = rect.left + 256;

				AdjustWindowRect(&rect, stye, FALSE);
				SetWindowPos(ddraw->hDraw, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			}
			else
				SetWindowPos(ddraw->hDraw, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SIZE:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw)
		{
			if (ddraw->hDraw)
				SetWindowPos(ddraw->hDraw, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

			if (ddraw->virtualMode)
			{
				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
				ddraw->viewport.refresh = TRUE;
			}

			SetEvent(ddraw->hDrawEvent);
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_DISPLAYCHANGE:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw)
		{
			DEVMODE devMode;
			MemoryZero(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_ACTIVATE:
	case WM_NCACTIVATE:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_ACTIVATEAPP:
	{
		if (!configDisplayWindowed)
		{
			OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
			if (ddraw && ddraw->virtualMode)
			{
				if ((BOOL)wParam)
				{
					ddraw->SetFullscreenMode();
					ddraw->RenderStart();
				}
				else {
					ddraw->RenderStop();
					if (!ddraw->isFinish)
						ChangeDisplaySettings(NULL, NULL);
				}
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	case WM_SYSKEYDOWN:
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

				OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
				if (ddraw)
				{
					if (!configDisplayWindowed)
					{
						if (ddraw->realMode)
						{
							if (ddraw->isStylesLoaded)
								GetWindowPlacement(hWnd, &ddraw->windowPlacement);
							ddraw->SetFullscreenMode();

							if (!ddraw->isFinish)
							{
								ddraw->RenderStop();
								ddraw->RenderStart();
							}
						}
					}
					else
					{
						if (ddraw->virtualMode)
						{
							ChangeDisplaySettings(NULL, NULL);
							ddraw->SetWindowedMode();

							if (!ddraw->isFinish)
							{
								ddraw->RenderStop();
								ddraw->RenderStart();
							}
						}
					}

					ddraw->isStateChanged = TRUE;

					ddraw->CalcView();
					ddraw->ScaleMouseOut(&mousePos);
					POINT point = { GET_X_LPARAM(mousePos), GET_Y_LPARAM(mousePos) };
					ClientToScreen(ddraw->hWnd, &point);
					SetCursorPos(point.x, point.y);
				}

				return NULL;
			}

			// FPS counter on/off
			case 'I':
			{
				configFpsCounter = !configFpsCounter;
				Config::Set(CONFIG_FPS, CONFIG_FPS_COUNTER, configFpsCounter);
				isFpsChanged = TRUE;
				return NULL;
			}

			// Filtering on/off
			case 'F':
			{
				configGlFiltering = configGlFiltering == GL_LINEAR ? GL_NEAREST : GL_LINEAR;
				Config::Set(CONFIG_GL, CONFIG_GL_FILTERING, configGlFiltering == GL_LINEAR);

				OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
				if (ddraw)
					ddraw->isStateChanged = TRUE;
				return NULL;
			}

			default:
				break;
			}
		}

		switch (wParam)
		{
		case 'W':
			wParam = VK_UP;
			break;

		case 'S':
			wParam = VK_DOWN;
			break;

		case 'A':
			wParam = VK_LEFT;
			break;

		case 'D':
			wParam = VK_RIGHT;
			break;

		default: break;
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case 'W':
			wParam = VK_UP;
			break;

		case 'S':
			wParam = VK_DOWN;
			break;

		case 'A':
			wParam = VK_LEFT;
			break;

		case 'D':
			wParam = VK_RIGHT;
			break;

		default: break;
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}


	case WM_CHAR:
	{
		switch (wParam)
		{
		case 'w':
		case 's':
		case 'a':
		case 'd':
		case 'W':
		case 'S':
		case 'A':
		case 'D':
			return NULL;
		default:
			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		}
	}

	case WM_SYSCOMMAND:
	{
		if (wParam == SC_KEYMENU && (lParam >> 16) <= 0)
			return NULL;

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

		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
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
		if (configDisplayWindowed && Main::FindDrawByWindow(hWnd))
			while (ShowCursor(TRUE) <= 0);

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	default:
		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}
}

LRESULT __stdcall PanelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
		if (ddraw && configDisplayWindowed)
			return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
		return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SYSCOMMAND:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	{
		if (!configDisplayWindowed)
			return WindowProc(GetParent(hWnd), uMsg, wParam, lParam);

		return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
	}

	default:
		return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
	}
}

VOID __fastcall UseShaderProgram(ShaderProgram* program)
{
	if (!program->id)
	{
		program->id = GLCreateProgram();

		GLuint vShader = GL::CompileShaderSource(program->vertexName, GL_VERTEX_SHADER);
		GLuint fShader = GL::CompileShaderSource(program->fragmentName, GL_FRAGMENT_SHADER);
		{

			GLAttachShader(program->id, vShader);
			GLAttachShader(program->id, fShader);
			{
				GLLinkProgram(program->id);
			}
			GLDetachShader(program->id, fShader);
			GLDetachShader(program->id, vShader);
		}
		GLDeleteShader(fShader);
		GLDeleteShader(vShader);

		GLUseProgram(program->id);
		GLUniformMatrix4fv(GLGetUniformLocation(program->id, "mvp"), 1, GL_FALSE, program->mvp);
		GLUniform1i(GLGetUniformLocation(program->id, "isscl"), program->interlaced);
		GLUniform1i(GLGetUniformLocation(program->id, "tex01"), 0);
		GLUniform1i(GLGetUniformLocation(program->id, "scl01"), 1);
	}
	else
		GLUseProgram(program->id);
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->hDc = ::GetDC(ddraw->hDraw);
	{
		PIXELFORMATDESCRIPTOR pfd;
		GL::PreparePixelFormatDescription(&pfd);
		if (!glPixelFormat && !GL::PreparePixelFormat(&pfd))
		{
			glPixelFormat = ChoosePixelFormat(ddraw->hDc, &pfd);
			if (!glPixelFormat)
				Main::ShowError("ChoosePixelFormat failed", __FILE__, __LINE__);
			else if (pfd.dwFlags & PFD_NEED_PALETTE)
				Main::ShowError("Needs palette", __FILE__, __LINE__);
		}

		if (!SetPixelFormat(ddraw->hDc, glPixelFormat, &pfd))
			Main::ShowError("SetPixelFormat failed", __FILE__, __LINE__);

		MemoryZero(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		if (DescribePixelFormat(ddraw->hDc, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == NULL)
			Main::ShowError("DescribePixelFormat failed", __FILE__, __LINE__);

		if ((pfd.iPixelType != PFD_TYPE_RGBA) ||
			(pfd.cRedBits < 5) || (pfd.cGreenBits < 5) || (pfd.cBlueBits < 5))
			Main::ShowError("Bad pixel type", __FILE__, __LINE__);

		HGLRC hRc = WGLCreateContext(ddraw->hDc);
		{
			WGLMakeCurrent(ddraw->hDc, hRc);
			{
				GL::CreateContextAttribs(ddraw->hDc, &hRc);

				DWORD glMaxTexSize;
				GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);

				if (glVersion >= GL_VER_3_0)
				{
					DWORD maxSize = ddraw->virtualMode->dwWidth > ddraw->virtualMode->dwHeight ? ddraw->virtualMode->dwWidth : ddraw->virtualMode->dwHeight;

					DWORD maxTexSize = 1;
					while (maxTexSize < maxSize)
						maxTexSize <<= 1;

					if (maxTexSize > glMaxTexSize)
						glVersion = GL_VER_1_4;
				}

				ddraw->fpsCounter = new FpsCounter(FPS_ACCURACY);
				{
					ddraw->textRenderer = new TextRenderer(ddraw->hDc, ddraw->virtualMode->dwWidth, ddraw->virtualMode->dwHeight);
					{
						ddraw->mult = ddraw->virtualMode->dwWidth / 320 - 1;
						DWORD fontSize = ddraw->mult ? 24 : 13;
						ddraw->hFontSubtitles = CreateFont(fontSize, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Georgia");
						{
							ddraw->hFontFps = CreateFont(fontSize, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Georgia");
							{
								if (configGlVersion == GL_VER_3)
								{
									if (glVersion >= GL_VER_3_0)
										ddraw->RenderNew();
									else
										Main::ShowError("OpenGL 3.0 is not supported", __FILE__, __LINE__);
								}
								else if (configGlVersion == GL_VER_1)
									ddraw->RenderOld();
								else if (glVersion >= GL_VER_3_0)
									ddraw->RenderNew();
								else
									ddraw->RenderOld();
							}
							DeleteObject(ddraw->hFontFps);
						}
						DeleteObject(ddraw->hFontSubtitles);
					}
					delete ddraw->textRenderer;
				}
				delete ddraw->fpsCounter;
			}
			WGLMakeCurrent(ddraw->hDc, NULL);
		}
		WGLDeleteContext(hRc);
	}
	::ReleaseDC(ddraw->hDraw, ddraw->hDc);

	return NULL;
}

OpenDrawSurface* OpenDraw::PreRender()
{
	OpenDrawSurface* surface = this->attachedSurface;
	if (surface)
	{
		if (configFpsCounter)
		{
			if (isFpsChanged)
			{
				isFpsChanged = FALSE;
				this->fpsCounter->Reset();
			}

			this->fpsCounter->Calculate();
		}

		DWORD dibSize = this->virtualMode->dwWidth * this->virtualMode->dwHeight;
		if (this->virtualMode->dwBPP == 8)
		{
			BYTE* src = surface->indexBuffer;
			PALETTEENTRY* dst = (PALETTEENTRY*)this->textRenderer->dibData;
			do
				*dst++ = surface->attachedPallete->entries[*src++];
			while (--dibSize);
		}
		else if (this->virtualMode->dwBPP == 16)
		{
			WORD* src = (WORD*)surface->indexBuffer;
			DWORD* dst = (DWORD*)this->textRenderer->dibData;

			do
			{
				WORD px = *src++;
				*dst++ = ((px & 0x7C00) >> 7) | ((px & 0x3E0) << 6) | ((px & 0x1F) << 19);
			} while (--dibSize);
		}
		else
			MemoryCopy(this->textRenderer->dibData, surface->indexBuffer, dibSize * sizeof(DWORD));

		// Draw Subtitles
		RECT rcText;
		if (activeSoundBuffer)
		{
			SubtitlesItem* currentSub = subtitlesCurrent;
			if (currentSub)
			{
				DWORD tick = GetTickCount() - soundStartTime;

				SubtitlesLine* currentLine = currentSub->lines + currentSub->count - 1;
				DWORD count = currentSub->count;
				while (count--)
				{
					if (tick >= currentLine->startTick)
					{
						if (tick <= currentLine->endTick)
						{
							if ((currentSub->type == SubtitlesWalk || currentSub->type == SubtitlesDescription) && !currentSub->flags)
								SetRect(&rcText, 10 << this->mult, (7 << this->mult), (LONG)this->virtualMode->dwHeight - (10 << this->mult), (LONG)this->virtualMode->dwHeight - (7 << mult));
							else
								SetRect(&rcText, 10 << this->mult, (7 << this->mult), (LONG)this->virtualMode->dwWidth - (10 << this->mult), (LONG)this->virtualMode->dwHeight - (7 << mult));

							this->textRenderer->DrawW(currentLine->text, this->hFontSubtitles, RGB(255, 255, 255), &rcText, TR_CENTER | TR_BOTTOM | TR_SHADOW);
						}

						break;
					}

					--currentLine;
				}
			}
		}

		// Draw FPS counter
		if (configFpsCounter)
		{
			SetRect(&rcText, 10 << this->mult, 5 << this->mult, (LONG)this->virtualMode->dwWidth - (10 << this->mult), (LONG)this->virtualMode->dwHeight - (5 << mult));
			this->textRenderer->DrawA("FPS: ", this->hFontFps, RGB(255, 255, 255), &rcText, TR_LEFT | TR_TOP | TR_SHADOW | TR_CALCULATE);

			CHAR fpsText[16];
			StrPrint(fpsText, "%.1f", this->fpsCounter->GetValue());
			SetRect(&rcText, rcText.right, 5 << this->mult, (LONG)this->virtualMode->dwWidth - (10 << this->mult), (LONG)this->virtualMode->dwHeight - (5 << mult));
			this->textRenderer->DrawA(fpsText, this->hFontFps, RGB(255, 255, 0), &rcText, TR_LEFT | TR_TOP | TR_SHADOW);
		}

		// Check screenshot
		if (this->isTakeSnapshot)
		{
			this->isTakeSnapshot = FALSE;

			if (OpenClipboard(NULL))
			{
				EmptyClipboard();

				DWORD dataCount = this->viewport.rectangle.width * this->viewport.rectangle.height;
				DWORD dataSize = dataCount * 3;
				HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
				{
					VOID* data = GlobalLock(hMemory);
					{
						MemoryZero(data, sizeof(BITMAPINFOHEADER));

						BITMAPINFOHEADER* bmi = (BITMAPINFOHEADER*)data;
						bmi->biSize = sizeof(BITMAPINFOHEADER);
						bmi->biWidth = this->viewport.rectangle.width;
						bmi->biHeight = this->viewport.rectangle.height;
						bmi->biPlanes = 1;
						bmi->biBitCount = 24;
						bmi->biCompression = BI_RGB;
						bmi->biXPelsPerMeter = 1;
						bmi->biYPelsPerMeter = 1;

						BYTE* dst = (BYTE*)data + sizeof(BITMAPINFOHEADER);
						GLReadPixels(this->viewport.rectangle.x, this->viewport.rectangle.y, this->viewport.rectangle.width, this->viewport.rectangle.height, glCapsBGR ? GL_BGR_EXT : GL_RGB, GL_UNSIGNED_BYTE, dst);
						if (!glCapsBGR)
						{
							do
							{
								BYTE val = *dst;
								*dst = *(dst + 2);
								*(dst + 2) = val;
								dst += 3;
							} while (--dataCount);
						}

						MessageBeep(0);
					}
					GlobalUnlock(hMemory);

					SetClipboardData(CF_DIB, hMemory);
				}
				GlobalFree(hMemory);

				CloseClipboard();
			}
		}

		this->CheckView();
	}

	return surface;
}

VOID OpenDraw::RenderOld()
{
	DWORD glMaxTexSize;
	GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);
	if (glMaxTexSize < 256)
		glMaxTexSize = 256;

	DWORD size = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;

	DWORD maxAllow = 1;
	while (maxAllow < size) maxAllow <<= 1;

	DWORD maxTexSize = maxAllow < glMaxTexSize ? maxAllow : glMaxTexSize;

	DWORD framePerWidth = this->virtualMode->dwWidth / maxTexSize + (this->virtualMode->dwWidth % maxTexSize ? 1 : 0);
	DWORD framePerHeight = this->virtualMode->dwHeight / maxTexSize + (this->virtualMode->dwHeight % maxTexSize ? 1 : 0);
	DWORD frameCount = framePerWidth * framePerHeight;

	GLuint scalelineId = 0;
	{
		if (*flags.interlaced)
		{
			if (glCapsMultitex)
			{
				GLActiveTexture(GL_TEXTURE1);
				GLEnable(GL_TEXTURE_2D);
			}
			else
				GLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			GLGenTextures(1, &scalelineId);
			{
				GLBindTexture(GL_TEXTURE_2D, scalelineId);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				GLTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, glCapsMultitex ? GL_MODULATE : GL_REPLACE);

				DWORD scalelineData[] = {
					0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
					0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
					0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
					0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000
				};

				GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, GL_NONE, GL_RGBA, GL_UNSIGNED_BYTE, scalelineData);
			}

			if (glCapsMultitex)
				GLActiveTexture(GL_TEXTURE0);
		}

		Frame* frames = (Frame*)MemoryAlloc(frameCount * sizeof(Frame));
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

					frame->tSize.width = width == maxTexSize ? 1.0f : FLOAT((FLOAT)width / maxTexSize);
					frame->tSize.height = height == maxTexSize ? 1.0f : FLOAT((FLOAT)height / maxTexSize);

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

						if (this->virtualMode->dwBPP == 16 && glVersion > GL_VER_1_1)
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

			VOID* pixelBuffer = MemoryAlloc(maxTexSize * maxTexSize * sizeof(DWORD));
			{
				BOOL isVSync = FALSE;
				if (WGLSwapInterval)
				{
					WGLSwapInterval(0);
					if (configDisplayVSync)
					{
						WGLSwapInterval(1);
						isVSync = TRUE;
					}
				}

				do
				{
					OpenDrawSurface* surface = this->PreRender();
					if (surface)
					{
						BOOL updateFilter = this->isStateChanged;
						if (this->isStateChanged)
						{
							this->isStateChanged = FALSE;

							if (*flags.interlaced && glCapsMultitex)
							{
								GLActiveTexture(GL_TEXTURE1);
								GLBindTexture(GL_TEXTURE_2D, scalelineId);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
								GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
								GLActiveTexture(GL_TEXTURE0);
							}
						}

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

							if (frameCount > 1)
							{
								DWORD* pix = (DWORD*)pixelBuffer;
								for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
								{
									DWORD* idx = (DWORD*)this->textRenderer->dibData + y * this->virtualMode->dwWidth + frame->rect.x;
									MemoryCopy(pix, idx, frame->rect.width * sizeof(DWORD));
									pix += frame->rect.width;
								}

								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
							}
							else
								GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, this->textRenderer->dibData);

							if (*flags.interlaced && glCapsMultitex)
							{
								GLBegin(GL_TRIANGLE_FAN);
								{

									GLMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);
									GLMultiTexCoord2f(GL_TEXTURE1, (GLfloat)frame->rect.x, (GLfloat)frame->rect.y);
									GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->rect.y);

									GLMultiTexCoord2f(GL_TEXTURE0, frame->tSize.width, 0.0);
									GLMultiTexCoord2f(GL_TEXTURE1, (GLfloat)frame->vSize.width, (GLfloat)frame->rect.y);
									GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->rect.y);

									GLMultiTexCoord2f(GL_TEXTURE0, frame->tSize.width, frame->tSize.height);
									GLMultiTexCoord2f(GL_TEXTURE1, (GLfloat)frame->vSize.width, (GLfloat)frame->vSize.height);
									GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->vSize.height);

									GLMultiTexCoord2f(GL_TEXTURE0, 0.0, frame->tSize.height);
									GLMultiTexCoord2f(GL_TEXTURE1, (GLfloat)frame->rect.x, (GLfloat)frame->vSize.height);
									GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->vSize.height);
								}
								GLEnd();
							}
							else
							{
								GLBegin(GL_TRIANGLE_FAN);
								{
									GLTexCoord2f(0.0, 0.0);
									GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->rect.y);

									GLTexCoord2f(frame->tSize.width, 0.0);
									GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->rect.y);

									GLTexCoord2f(frame->tSize.width, frame->tSize.height);
									GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->vSize.height);

									GLTexCoord2f(0.0, frame->tSize.height);
									GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->vSize.height);
								}
								GLEnd();

								if (*flags.interlaced)
								{
									GLEnable(GL_BLEND);
									{
										GLBindTexture(GL_TEXTURE_2D, scalelineId);
										GLBegin(GL_TRIANGLE_FAN);
										{
											GLTexCoord2f((GLfloat)frame->rect.x, (GLfloat)frame->rect.y);
											GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->rect.y);

											GLTexCoord2f((GLfloat)frame->vSize.width, (GLfloat)frame->rect.y);
											GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->rect.y);

											GLTexCoord2f((GLfloat)frame->vSize.width, (GLfloat)frame->vSize.height);
											GLVertex2f((GLfloat)frame->vSize.width, (GLfloat)frame->vSize.height);

											GLTexCoord2f((GLfloat)frame->rect.x, (GLfloat)frame->vSize.height);
											GLVertex2f((GLfloat)frame->rect.x, (GLfloat)frame->vSize.height);
										}
										GLEnd();
									}
									GLDisable(GL_BLEND);
								}
							}

							++frame;
						}

						SwapBuffers(hDc);
						WaitForSingleObject(this->hDrawEvent, 66);
						if (isVSync)
							GLFinish();
					}
				} while (!this->isFinish);
			}
			MemoryFree(pixelBuffer);

			// Remove
			frame = frames;
			DWORD count = frameCount;
			while (count--)
			{
				GLDeleteTextures(2, frame->id);
				++frame;
			}
		}
		MemoryFree(frames);
	}
	if (scalelineId)
		GLDeleteTextures(1, &scalelineId);
}

VOID OpenDraw::RenderNew()
{
	DWORD maxSize = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;
	DWORD maxTexSize = 1;
	while (maxTexSize < maxSize) maxTexSize <<= 1;
	FLOAT texWidth = this->virtualMode->dwWidth == maxTexSize ? 1.0f : FLOAT((FLOAT)this->virtualMode->dwWidth / maxTexSize);
	FLOAT texHeight = this->virtualMode->dwHeight == maxTexSize ? 1.0f : FLOAT((FLOAT)this->virtualMode->dwHeight / maxTexSize);

	FLOAT buffer[4][4] = {
		{ 0.0, 0.0,																	0.0, 0.0 },
		{ (FLOAT)this->virtualMode->dwWidth, 0.0,									texWidth, 0.0 },
		{ (FLOAT)this->virtualMode->dwWidth, (FLOAT)this->virtualMode->dwHeight,	texWidth, texHeight },
		{ 0.0, (FLOAT)this->virtualMode->dwHeight,									0.0, texHeight }
	};

	FLOAT mvpMatrix[4][4] = {
		{ 2.0f / this->virtualMode->dwWidth, 0.0f, 0.0f, 0.0f },
		{ 0.0f, -2.0f / this->virtualMode->dwHeight, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};

	struct TextureIds
	{
		GLuint normalId;
		GLuint scalelineId;
	} textures;

	GLuint arrayName, bufferName;

	ShaderProgramsList shaders = {
		0, IDR_VERTEX, IDR_FRAGMENT_NEAREST, (GLfloat*)mvpMatrix, *flags.interlaced,
		0, IDR_VERTEX, IDR_FRAGMENT_BICUBIC, (GLfloat*)mvpMatrix, *flags.interlaced
	};

	GLGenTextures(sizeof(textures) / sizeof(GLuint), (GLuint*)&textures);
	{
		if (*flags.interlaced)
		{
			GLActiveTexture(GL_TEXTURE1);
			{
				GLBindTexture(GL_TEXTURE_2D, textures.scalelineId);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
				GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);

				INT scalelineData[] = { -1, -1, 0, 0 };
				GLTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4, 4, GL_NONE, GL_RED, GL_UNSIGNED_BYTE, scalelineData);
			}
		}

		GLActiveTexture(GL_TEXTURE0);
		{
			GLBindTexture(GL_TEXTURE_2D, textures.normalId);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

						ShaderProgram* program = configGlFiltering == GL_LINEAR ? &shaders.bicubic : &shaders.nearest;

						UseShaderProgram(program);

						GLint attrCoordsLoc = GLGetAttribLocation(program->id, "vCoord");
						GLEnableVertexAttribArray(attrCoordsLoc);
						GLVertexAttribPointer(attrCoordsLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
						{
							GLClearColor(0.0, 0.0, 0.0, 1.0);

							BOOL isVSync = FALSE;
							if (WGLSwapInterval)
							{
								WGLSwapInterval(0);
								if (configDisplayVSync)
								{
									WGLSwapInterval(1);
									isVSync = TRUE;
								}
							}

							do
							{
								OpenDrawSurface* surface = this->PreRender();
								if (surface)
								{
									if (this->isStateChanged)
									{
										this->isStateChanged = FALSE;
										UseShaderProgram(configGlFiltering == GL_LINEAR ? &shaders.bicubic : &shaders.nearest);

										if (*flags.interlaced)
										{
											GLActiveTexture(GL_TEXTURE1);
											GLBindTexture(GL_TEXTURE_2D, textures.scalelineId);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
											GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);

											GLActiveTexture(GL_TEXTURE0);
											GLBindTexture(GL_TEXTURE_2D, textures.normalId);
										}
									}

									GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_RGBA, GL_UNSIGNED_BYTE, this->textRenderer->dibData);

									GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

									SwapBuffers(hDc);
									WaitForSingleObject(this->hDrawEvent, 66);
									if (isVSync)
										GLFinish();
								}
							} while (!this->isFinish);
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
	GLDeleteTextures(sizeof(textures) / sizeof(GLuint), (GLuint*)&textures);

	GLUseProgram(NULL);

	ShaderProgram* shaderProgram = (ShaderProgram*)&shaders;
	DWORD count = sizeof(ShaderProgramsList) / sizeof(ShaderProgram);
	do
	{
		if (shaderProgram->id)
			GLDeleteProgram(shaderProgram->id);
	} while (--count);
}

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;
	GL::Load();

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (!configDisplayWindowed)
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
			WC_STATIC,
			NULL,
			WS_VISIBLE | WS_POPUP,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}
	else
	{
		this->hDraw = CreateWindowEx(
			WS_EX_CONTROLPARENT,
			WC_STATIC,
			NULL,
			WS_VISIBLE | WS_CHILD,
			rect.left, rect.top,
			rect.right - rect.left, rect.bottom - rect.top,
			this->hWnd,
			NULL,
			hDllModule,
			NULL);
	}

	OldPanelProc = (WNDPROC)SetWindowLongPtr(this->hDraw, GWLP_WNDPROC, (LONG_PTR)PanelProc);

	SetClassLongPtr(this->hDraw, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hDraw, NULL, NULL, RDW_INVALIDATE);
	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right - rect.left;
	this->viewport.height = rect.bottom - rect.top;
	this->viewport.refresh = TRUE;
	this->isStateChanged = TRUE;

	DWORD threadId;
	SECURITY_ATTRIBUTES sAttribs;
	MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
	sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	this->hDrawThread = CreateThread(&sAttribs, NULL, RenderThread, this, NORMAL_PRIORITY_CLASS, &threadId);
}

VOID OpenDraw::RenderStop()
{
	if (this->isFinish)
		return;

	this->isFinish = TRUE;
	SetEvent(this->hDrawEvent);
	WaitForSingleObject(this->hDrawThread, INFINITE);
	CloseHandle(this->hDrawThread);
	this->hDrawThread = NULL;

	BOOL wasFull = GetWindowLong(this->hDraw, GWL_STYLE) & WS_POPUP;
	if (DestroyWindow(this->hDraw))
		this->hDraw = NULL;

	if (wasFull)
		GL::ResetContext();

	ClipCursor(NULL);
}

OpenDraw::OpenDraw(OpenDraw* lastObj)
{
	this->last = lastObj;
	this->isStylesLoaded = NULL;
	this->realMode = NULL;
	this->virtualMode = NULL;
	this->attachedSurface = NULL;
	this->isTakeSnapshot = FALSE;

	this->hWnd = NULL;
	this->hDraw = NULL;

	this->surfaceEntries = NULL;
	this->paletteEntries = NULL;
	this->clipperEntries = NULL;

	this->hDrawEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

OpenDraw::~OpenDraw()
{
	OpenDrawSurface* surfaceEntry = this->surfaceEntries;
	while (surfaceEntry)
	{
		OpenDrawSurface* curr = surfaceEntry->prev;
		delete surfaceEntry;
		surfaceEntry = curr;
	}

	OpenDrawPalette* paletteEntry = this->paletteEntries;
	while (paletteEntry)
	{
		OpenDrawPalette* curr = paletteEntry->prev;
		delete paletteEntry;
		paletteEntry = curr;
	}

	OpenDrawClipper* clipperEntry = this->clipperEntries;
	while (clipperEntry)
	{
		OpenDrawClipper* curr = clipperEntry->prev;
		delete clipperEntry;
		clipperEntry = curr;
	}

	CloseHandle(this->hDrawEvent);
}

VOID OpenDraw::CalcView()
{
	this->viewport.rectangle.x = this->viewport.rectangle.y = 0;
	this->viewport.point.x = this->viewport.point.y = 0.0f;

	this->viewport.rectangle.width = this->viewport.width;
	this->viewport.rectangle.height = this->viewport.height;

	this->viewport.clipFactor.x = this->viewport.viewFactor.x = (FLOAT)this->viewport.width / (this->virtualMode->dwWidth << *flags.interlaced);
	this->viewport.clipFactor.y = this->viewport.viewFactor.y = (FLOAT)this->viewport.height / (this->virtualMode->dwHeight << *flags.interlaced);

	if (configDisplayAspect && this->viewport.viewFactor.x != this->viewport.viewFactor.y)
	{
		if (this->viewport.viewFactor.x > this->viewport.viewFactor.y)
		{
			FLOAT fw = this->viewport.viewFactor.y * (this->virtualMode->dwWidth << *flags.interlaced);
			this->viewport.rectangle.width = (DWORD)MathRound(fw);

			this->viewport.point.x = ((FLOAT)this->viewport.width - fw) / 2.0f;
			this->viewport.rectangle.x = (DWORD)MathRound(this->viewport.point.x);

			this->viewport.clipFactor.x = this->viewport.viewFactor.y;
		}
		else
		{
			FLOAT fh = this->viewport.viewFactor.x * (this->virtualMode->dwHeight << *flags.interlaced);
			this->viewport.rectangle.height = (DWORD)MathRound(fh);

			this->viewport.point.y = ((FLOAT)this->viewport.height - fh) / 2.0f;
			this->viewport.rectangle.y = (DWORD)MathRound(this->viewport.point.y);

			this->viewport.clipFactor.y = this->viewport.viewFactor.x;
		}
	}
}

VOID OpenDraw::CheckView()
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

HRESULT OpenDraw::SetFullscreenMode()
{
	DEVMODE devMode;
	MemoryZero(&devMode, sizeof(DEVMODE));
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

	RECT rect = { 0, 0, (LONG)this->realMode->dwWidth, (LONG)this->realMode->dwHeight };
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

HRESULT OpenDraw::SetWindowedMode()
{
	if (!this->isStylesLoaded)
	{
		this->isStylesLoaded = TRUE;
		MONITORINFO mi = { sizeof(mi) };
		HMONITOR hMon = MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hMon, &mi);
		DWORD monWidth = mi.rcWork.right - mi.rcWork.left;
		DWORD monHeight = mi.rcWork.bottom - mi.rcWork.top;
		DWORD newWidth = (DWORD)MathRound(0.75f * (mi.rcWork.right - mi.rcWork.left));
		DWORD newHeght = (DWORD)MathRound(0.75f * (mi.rcWork.bottom - mi.rcWork.top));

		FLOAT k = (FLOAT)this->virtualMode->dwWidth / this->virtualMode->dwHeight;

		DWORD check = (DWORD)MathRound((FLOAT)newHeght * k);
		if (newWidth > check)
			newWidth = check;
		else
			newHeght = (DWORD)MathRound((FLOAT)newWidth / k);

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

VOID OpenDraw::ScaleMouseIn(LPARAM* lParam)
{
	if (configDisplayWindowed || this->virtualMode != this->realMode)
	{
		INT xPos = GET_X_LPARAM(*lParam);
		INT yPos = GET_Y_LPARAM(*lParam);

		if (*flags.interlaced)
		{
			xPos >>= 1;
			yPos >>= 1;
		}

		if (xPos < (INT)this->viewport.rectangle.x)
			xPos = 0;
		else if (xPos >= (INT)this->viewport.rectangle.x + (INT)this->viewport.rectangle.width)
			xPos = (INT)this->virtualMode->dwWidth - 1;
		else
			xPos = (INT)MathRound((FLOAT)(xPos + 1 - (INT)this->viewport.rectangle.x) / this->viewport.clipFactor.x) - 1;

		if (yPos < (INT)this->viewport.rectangle.y)
			yPos = 0;
		else if (yPos >= (INT)this->viewport.rectangle.y + (INT)this->viewport.rectangle.height)
			yPos = (INT)this->virtualMode->dwHeight - 1;
		else
			yPos = (INT)MathRound((FLOAT)(yPos + 1 - (INT)this->viewport.rectangle.y) / this->viewport.clipFactor.y) - 1;

		*lParam = MAKELONG(xPos, yPos);
	}
}

VOID OpenDraw::ScaleMouseOut(LPARAM* lParam)
{
	if (configDisplayWindowed || this->virtualMode != this->realMode)
	{
		INT xPos = (INT)MathRound(this->viewport.clipFactor.x * (GET_X_LPARAM(*lParam) + 1)) - 1;
		if (xPos < (INT)this->viewport.rectangle.x)
			xPos = 0;
		else if (xPos >= (INT)this->viewport.rectangle.x + (INT)this->viewport.rectangle.width)
			xPos = (INT)this->viewport.rectangle.x + (INT)this->viewport.rectangle.width;

		INT yPos = (INT)MathRound(this->viewport.clipFactor.y * (GET_Y_LPARAM(*lParam) + 1)) - 1;
		if (yPos < (INT)this->viewport.rectangle.y)
			yPos = 0;
		else if (yPos >= (INT)this->viewport.rectangle.y + (INT)this->viewport.rectangle.height)
			yPos = (INT)this->viewport.rectangle.y + (INT)this->viewport.rectangle.height;

		if (*flags.interlaced)
		{
			xPos <<= 1;
			yPos <<= 1;
		}

		*lParam = MAKELONG(xPos, yPos);
	}
}

VOID OpenDraw::CheckDisplayMode()
{
	DisplayMode* mode = this->virtualMode;
	if (configDisplayResolution.index)
		this->realMode = &modesList[9 + (mode->dwBPP == 32 ? 2 : (mode->dwBPP == 16 ? 1 : 0))];
	else
	{
		if (*flags.interlaced)
			mode += 3;

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
ULONG OpenDraw::Release()
{
	if (ddrawList == this)
		ddrawList = NULL;
	else
	{
		OpenDraw* ddraw = ddrawList;
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

HRESULT OpenDraw::CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
	this->clipperEntries = new OpenDrawClipper(this);
	*lplpDDClipper = (LPDIRECTDRAWCLIPPER)this->clipperEntries;

	return DD_OK;
}

HRESULT OpenDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE* lplpDDPalette, IUnknown* pUnkOuter)
{
	this->paletteEntries = new OpenDrawPalette(this);
	*lplpDDPalette = (LPDIRECTDRAWPALETTE)this->paletteEntries;

	this->paletteEntries->SetEntries(0, 0, 256, lpDDColorArray);

	return DD_OK;
}

HRESULT OpenDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
	OpenDrawSurface* surface = new OpenDrawSurface(this, TRUE);
	*lplpDDSurface = (LPDIRECTDRAWSURFACE)surface;

	if (this->virtualMode)
	{
		lpDDSurfaceDesc->dwWidth = this->virtualMode->dwWidth;
		lpDDSurfaceDesc->dwHeight = this->virtualMode->dwHeight;
		lpDDSurfaceDesc->lPitch = this->virtualMode->dwWidth * (this->virtualMode->dwBPP >> 3);
	}

	this->RenderStart();

	return DD_OK;
}

HRESULT OpenDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
	DisplayMode* mode = modesList;
	DWORD i;
	for (i = 0; i < 3; ++i)
	{
		for (DWORD j = 0; j < 3; ++j, ++mode)
		{
			mode->dwWidth = 320 << i;
			mode->dwHeight = 240 << i;
			mode->dwBPP = 8 << j;
			mode->dwExists = FALSE;
		}
	}

	DEVMODE devMode;
	MemoryZero(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);

	DWORD bppCheck = 32;
	if (configDisplayResolution.index > 1)
		bppCheck = configDisplayResolution.bpp;
	else if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
	{
		bppCheck = devMode.dmBitsPerPel;
		MemoryZero(&devMode, sizeof(DEVMODE));
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

		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
	}

	MemoryZero(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);
	if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode))
	{
		mode = &modesList[9];
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

	DDSURFACEDESC ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC));
	for (i = 0; i < 3; ++i)
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

HRESULT OpenDraw::GetCaps(LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
	lpDDDriverCaps->dwCaps = DDCAPS_3D;
	return DD_OK;
}

HRESULT OpenDraw::RestoreDisplayMode()
{
	ChangeDisplaySettings(NULL, NULL);
	return DD_OK;
}

HRESULT OpenDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
	if (hWnd && hWnd != this->hWnd)
	{
		this->hWnd = hWnd;
		this->windowPlacement.length = sizeof(WINDOWPLACEMENT);
		this->mbPressed = NULL;

		if (!blackBrush)
			blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

		if (!OldWindowProc)
			OldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);
	}

	return DD_OK;
}

HRESULT OpenDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	DWORD idx;
	if (dwWidth == 320 && dwHeight == 240)
		idx = 0;
	else if (dwWidth == 640 && dwHeight == 480)
		idx = 3;
	else if (dwWidth == 1280 && dwHeight == 960)
		idx = 6;
	else
		idx = 9;

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

	return DD_OK;
}