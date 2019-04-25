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
#include "resource.h"
#include "math.h"
#include "Windowsx.h"
#include "OpenDraw.h"
#include "FpsCounter.h"
#include "Config.h"
#include "Main.h"
#include "CommCtrl.h"
#include "TextRenderer.h"

#define MIN_WIDTH 240
#define MIN_HEIGHT 180

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

DisplayMode modesList[12];

WNDPROC OldWindowProc, OldPanelProc;
LPARAM mousePos;
HWND mousehWnd;

LRESULT __stdcall WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND:
	{
		if (!configDisplayWindowed)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			return TRUE;
		}
		return NULL;
	}

	case WM_GETMINMAXINFO:
	{
		if (configDisplayWindowed)
		{
			RECT rect = { 0, 0, MIN_WIDTH, MIN_HEIGHT };
			AdjustWindowRect(&rect, GetWindowLong(hWnd, GWL_STYLE), FALSE);

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rect.right - rect.left;
			mmi->ptMinTrackSize.y = rect.bottom - rect.top;
			mmi->ptMaxTrackSize.x = LONG_MAX >> 16;
			mmi->ptMaxTrackSize.y = LONG_MAX >> 16;
			mmi->ptMaxSize.x = LONG_MAX >> 16;
			mmi->ptMaxSize.y = LONG_MAX >> 16;

			return NULL;
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_MOVE:
	{
		OpenDraw* ddraw = ddrawList;
		if (ddraw && ddraw->hDraw && !configSingleWindow)
		{
			if (!configDisplayWindowed)
			{
				POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ScreenToClient(hWnd, &point);

				RECT rect;
				rect.left = point.x - GET_X_LPARAM(lParam);
				rect.top = point.y - GET_Y_LPARAM(lParam);
				rect.right = rect.left + 256;
				rect.bottom = rect.left + 256;

				AdjustWindowRect(&rect, FS_STYLE, FALSE);
				SetWindowPos(ddraw->hDraw, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			}
			else
				SetWindowPos(ddraw->hDraw, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SIZE:
	{
		OpenDraw* ddraw = ddrawList;
		if (ddraw)
		{
			if (ddraw->hDraw && !configSingleWindow)
				SetWindowPos(ddraw->hDraw, NULL, 0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

			if (ddraw->virtualMode)
			{
				ddraw->viewport.width = LOWORD(lParam);
				ddraw->viewport.height = HIWORD(lParam);
				ddraw->viewport.refresh = TRUE;
			}

			ddraw->SetSyncDraw();
		}

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_DISPLAYCHANGE:
	{
		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
		configFpsSync = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && (devMode.dmFields & DM_DISPLAYFREQUENCY) ? 1.0f / devMode.dmDisplayFrequency : 0.0;

		return CallWindowProc(OldWindowProc, hWnd, uMsg, wParam, lParam);
	}

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_ACTIVATE:
	case WM_NCACTIVATE:
		if (LOWORD(wParam))
			SendMessage(hWnd, WM_KEYUP, VK_MENU, NULL);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_ACTIVATEAPP:
	{
		if (!configDisplayWindowed)
		{
			OpenDraw* ddraw = ddrawList;
			if (ddraw && ddraw->virtualMode)
			{
				if ((BOOL)wParam)
				{
					ddraw->SetFullscreenMode();
					ddraw->RenderStart();
				}
				else
				{
					ddraw->RenderStop();
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

				OpenDraw* ddraw = ddrawList;
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

				OpenDraw* ddraw = ddrawList;
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

		OpenDraw* ddraw = ddrawList;
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
		if (configDisplayWindowed && ddrawList)
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
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_SYSCOMMAND:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		return WindowProc(GetParent(hWnd), uMsg, wParam, lParam);

	default:
		return CallWindowProc(OldPanelProc, hWnd, uMsg, wParam, lParam);
	}
}

VOID __fastcall UseShaderProgram(ShaderProgram* program, DWORD texSize)
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
		GLUniform1i(GLGetUniformLocation(program->id, "tex01"), GL_TEXTURE0 - GL_TEXTURE0);
		GLUniform1i(GLGetUniformLocation(program->id, "scl01"), GL_TEXTURE1 - GL_TEXTURE0);

		GLint loc = GLGetUniformLocation(program->id, "texSize");
		if (loc >= 0)
			GLUniform2f(loc, (FLOAT)texSize, (FLOAT)texSize);
	}
	else
		GLUseProgram(program->id);
}

DWORD __stdcall RenderThread(LPVOID lpParameter)
{
	OpenDraw* ddraw = (OpenDraw*)lpParameter;
	ddraw->RenderStartInternal();
	{
		if (!configDisplayVSync || ddraw->sceneData->isVSync)
		{
			do
				ddraw->RenderFrame();
			while (!ddraw->isFinish);
		}
		else
		{
			do
			{
				if (configFpsSync != 0.0)
				{
					LONGLONG qp;
					QueryPerformanceFrequency((LARGE_INTEGER*)&qp);
					DOUBLE timerResolution = (DOUBLE)qp;

					QueryPerformanceCounter((LARGE_INTEGER*)&qp);
					DOUBLE currTime = (DOUBLE)qp / timerResolution;

					if (currTime >= ddraw->nextSyncTime)
					{
						ddraw->nextSyncTime += configFpsSync * (1.0f + (FLOAT)DWORD((currTime - ddraw->nextSyncTime) / configFpsSync));
						ddraw->RenderFrame();
					}
				}
			} while (!ddraw->isFinish);
		}
	}
	ddraw->RenderStopinternal();
	return NULL;
}

// ------------------------------------------------

VOID OpenDraw::RenderStart()
{
	if (!this->isFinish || !this->hWnd)
		return;

	this->isFinish = FALSE;

	RECT rect;
	GetClientRect(this->hWnd, &rect);

	if (configSingleWindow)
		this->hDraw = this->hWnd;
	else
	{
		if (!configDisplayWindowed)
		{
			this->hDraw = CreateWindowEx(
				WS_EX_CONTROLPARENT | WS_EX_TOPMOST,
				WC_DRAW,
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
				WC_DRAW,
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
	}

	SetClassLongPtr(this->hWnd, GCLP_HBRBACKGROUND, NULL);
	RedrawWindow(this->hWnd, NULL, NULL, RDW_INVALIDATE);

	this->viewport.width = rect.right - rect.left;
	this->viewport.height = rect.bottom - rect.top;
	this->viewport.refresh = TRUE;
	this->isStateChanged = TRUE;

	if (configSingleThread)
		this->RenderStartInternal();
	else
		this->hDrawThread = CreateThread(NULL, NULL, RenderThread, this, NORMAL_PRIORITY_CLASS, NULL);
}

VOID OpenDraw::RenderStop()
{
	if (this->isFinish)
		return;

	this->isFinish = TRUE;

	if (configSingleThread)
		this->RenderStopinternal();
	else
	{
		SetEvent(this->hDrawEvent);
		WaitForSingleObject(this->hDrawThread, INFINITE);
		CloseHandle(this->hDrawThread);
		this->hDrawThread = NULL;
	}

	if (!configSingleWindow)
	{
		if (DestroyWindow(this->hDraw))
			this->hDraw = NULL;

		if (!configDisplayWindowed)
			GL::ResetContext();
	}

	ClipCursor(NULL);
}

// ------------------------------------------------

VOID OpenDraw::RenderStartScene()
{
	if (this->pRenderStartScene)
		(this->*pRenderStartScene)();
}

VOID OpenDraw::RenderFrame()
{
	if (this->pRenderFrame)
		(this->*pRenderFrame)();
}

VOID OpenDraw::RenderEndScene()
{
	if (this->pRenderEndScene)
		(this->*pRenderEndScene)();
}

// ------------------------------------------------

VOID OpenDraw::RenderStartInternal()
{
	this->hDc = ::GetDC(this->hDraw);
	if (this->hDc)
	{
		GL::SetPixelFormat(this->hDc);

		this->hRc = WGLCreateContext(this->hDc);
		if (this->hRc)
		{
			if (WGLMakeCurrent(this->hDc, this->hRc))
			{
				GL::CreateContextAttribs(this->hDc, &this->hRc);

				DWORD glMaxTexSize;
				GLGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&glMaxTexSize);

				if (glVersion >= GL_VER_3_0)
				{
					DWORD maxSize = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;

					DWORD maxTexSize = 1;
					while (maxTexSize < maxSize)
						maxTexSize <<= 1;

					if (maxTexSize > glMaxTexSize)
						glVersion = GL_VER_1_4;
				}

				this->fpsCounter = new FpsCounter(FPS_ACCURACY);
				if (this->fpsCounter)
				{
					this->textRenderer = new TextRenderer(this->hDc, this->virtualMode->dwWidth, this->virtualMode->dwHeight);
					if (this->textRenderer)
					{
						this->mult = this->virtualMode->dwWidth / 320 - 1;
						this->subtitlesFont = &subtitlesFonts[this->mult];
						this->hFontFps = CreateFont(this->mult ? 24 : 13, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Georgia");
						if (this->hFontFps)
						{
							BOOL isNew = FALSE;

							if (configGlVersion == GL_VER_3)
							{
								if (glVersion >= GL_VER_3_0)
									isNew = TRUE;
								else
									Main::ShowError("OpenGL 3.0 is not supported", __FILE__, __LINE__);
							}
							else if (configGlVersion != GL_VER_1 && glVersion >= GL_VER_3_0)
								isNew = TRUE;

							if (!isNew)
							{
								this->pRenderStartScene = &OpenDraw::RenderStartSceneOld;
								this->pRenderFrame = &OpenDraw::RenderFrameOld;
								this->pRenderEndScene = &OpenDraw::RenderEndSceneOld;
							}
							else
							{
								this->pRenderStartScene = &OpenDraw::RenderStartSceneNew;
								this->pRenderFrame = &OpenDraw::RenderFrameNew;
								this->pRenderEndScene = &OpenDraw::RenderEndSceneNew;
							}

							this->RenderStartScene();
						}
					}
				}
			}
		}
	}
}

VOID OpenDraw::RenderStopinternal()
{
	if (this->hDc)
	{
		if (this->hRc)
		{
			if (this->fpsCounter)
			{
				if (this->textRenderer)
				{
					if (this->hFontFps)
					{
						this->RenderEndScene();

						this->pRenderStartScene = NULL;
						this->pRenderEndScene = NULL;
						this->pRenderFrame = NULL;

						DeleteObject(this->hFontFps);
						this->hFontFps = NULL;
					}

					delete this->textRenderer;
					this->textRenderer = NULL;
				}
				delete this->fpsCounter;
				this->fpsCounter = NULL;
			}

			WGLMakeCurrent(this->hDc, NULL);
			WGLDeleteContext(this->hRc);
			this->hRc = NULL;
		}

		::ReleaseDC(this->hDraw, this->hDc);
		this->hDc = NULL;
	}
}

// ------------------------------------------------

VOID OpenDraw::CheckVSync()
{
	this->sceneData->isVSync = FALSE;
	if (glCapsVSync)
	{
		BOOL isDWM;
		this->sceneData->isVSync = configDisplayVSync && (!configDisplayWindowed || !IsDwmCompositionEnabled || IsDwmCompositionEnabled(&isDWM) == S_OK && !isDWM);
		WGLSwapInterval(this->sceneData->isVSync ? glCapsVSync : 0);
	}
}

VOID OpenDraw::SetSyncDraw()
{
	if (!configSingleThread)
	{
		SetEvent(this->hDrawEvent);
		Sleep(0);
	}
	else
		this->hDrawFlag = TRUE;
}

VOID OpenDraw::CheckSyncDraw()
{
	if (configDisplayVSync && configFpsSync != 0.0)
	{
		LONGLONG qp;
		QueryPerformanceFrequency((LARGE_INTEGER*)&qp);
		DOUBLE timerResolution = (DOUBLE)qp;

		QueryPerformanceCounter((LARGE_INTEGER*)&qp);
		DOUBLE currTime = (DOUBLE)qp / timerResolution;

		if (currTime >= this->nextSyncTime)
		{
			this->nextSyncTime += configFpsSync * (0.001f + (FLOAT)DWORD((currTime - this->nextSyncTime) / configFpsSync));

			if (this->hDrawFlag)
			{
				this->hDrawFlag = FALSE;
				this->RenderFrame();
			}
		}
	}
	else if (this->hDrawFlag)
	{
		this->hDrawFlag = FALSE;
		this->RenderFrame();
	}
}

VOID OpenDraw::RenderStartSceneOld()
{
	SceneDataOld* renderData = (SceneDataOld*)MemoryAlloc(sizeof(SceneDataOld));
	MemoryZero(renderData, sizeof(SceneDataOld));
	this->sceneData = renderData;

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
	renderData->frameCount = framePerWidth * framePerHeight;

	renderData->scalelineId = 0;
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

			GLGenTextures(1, &renderData->scalelineId);
			{
				GLBindTexture(GL_TEXTURE_2D, renderData->scalelineId);
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

		renderData->frames = (Frame*)MemoryAlloc(renderData->frameCount * sizeof(Frame));
		{
			Frame* frame = renderData->frames;
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

			renderData->pixelBuffer = MemoryAlloc(maxTexSize * maxTexSize * sizeof(DWORD));

			this->CheckVSync();
		}
	}
}

VOID OpenDraw::RenderStartSceneNew()
{
	SceneDataNew* renderData = (SceneDataNew*)MemoryAlloc(sizeof(SceneDataNew));
	MemoryZero(renderData, sizeof(SceneDataNew));
	this->sceneData = renderData;

	DWORD maxSize = this->virtualMode->dwWidth > this->virtualMode->dwHeight ? this->virtualMode->dwWidth : this->virtualMode->dwHeight;
	renderData->maxTexSize = 1;
	while (renderData->maxTexSize < maxSize) renderData->maxTexSize <<= 1;
	FLOAT texWidth = this->virtualMode->dwWidth == renderData->maxTexSize ? 1.0f : FLOAT((FLOAT)this->virtualMode->dwWidth / renderData->maxTexSize);
	FLOAT texHeight = this->virtualMode->dwHeight == renderData->maxTexSize ? 1.0f : FLOAT((FLOAT)this->virtualMode->dwHeight / renderData->maxTexSize);

	FLOAT bufferTemp[4][4] = {
		{ 0.0, 0.0,																	0.0, 0.0 },
		{ (FLOAT)this->virtualMode->dwWidth, 0.0,									texWidth, 0.0 },
		{ (FLOAT)this->virtualMode->dwWidth, (FLOAT)this->virtualMode->dwHeight,	texWidth, texHeight },
		{ 0.0, (FLOAT)this->virtualMode->dwHeight,									0.0, texHeight }
	};
	MemoryCopy(renderData->buffer, bufferTemp, sizeof(bufferTemp));

	FLOAT mvpTemp[4][4] = {
		{ FLOAT(2.0f / this->virtualMode->dwWidth), 0.0f, 0.0f, 0.0f },
		{ 0.0f, FLOAT(-2.0f / this->virtualMode->dwHeight), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 2.0f, 0.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f }
	};
	MemoryCopy(renderData->mvpMatrix, mvpTemp, sizeof(mvpTemp));

	renderData->shaders.nearest.id = 0;
	renderData->shaders.nearest.vertexName = IDR_VERTEX_NEAREST;
	renderData->shaders.nearest.fragmentName = IDR_FRAGMENT_NEAREST;
	renderData->shaders.nearest.mvp = (GLfloat*)renderData->mvpMatrix;
	renderData->shaders.nearest.interlaced = *flags.interlaced;

	renderData->shaders.bicubic.id = 0;
	renderData->shaders.bicubic.vertexName = IDR_VERTEX_CUBIC;
	renderData->shaders.bicubic.fragmentName = IDR_FRAGMENT_CUBIC;
	renderData->shaders.bicubic.mvp = (GLfloat*)renderData->mvpMatrix;
	renderData->shaders.bicubic.interlaced = *flags.interlaced;

	GLGenTextures(sizeof(renderData->textures) / sizeof(GLuint), (GLuint*)&renderData->textures);
	{
		if (*flags.interlaced)
		{
			GLActiveTexture(GL_TEXTURE1);
			{
				GLBindTexture(GL_TEXTURE_2D, renderData->textures.scalelineId);
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
			GLBindTexture(GL_TEXTURE_2D, renderData->textures.normalId);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glCapsClampToEdge);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glCapsClampToEdge);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			GLTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderData->maxTexSize, renderData->maxTexSize, GL_NONE, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}

		GLGenVertexArrays(1, &renderData->arrayName);
		{
			GLBindVertexArray(renderData->arrayName);
			{
				GLGenBuffers(1, &renderData->bufferName);
				{
					GLBindBuffer(GL_ARRAY_BUFFER, renderData->bufferName);
					{
						GLBufferData(GL_ARRAY_BUFFER, sizeof(renderData->buffer), renderData->buffer, GL_STREAM_DRAW);

						ShaderProgram* program = configGlFiltering == GL_LINEAR ? &renderData->shaders.bicubic : &renderData->shaders.nearest;

						UseShaderProgram(program, renderData->maxTexSize);

						GLint attrCoordsLoc = GLGetAttribLocation(program->id, "vCoord");
						GLEnableVertexAttribArray(attrCoordsLoc);
						GLVertexAttribPointer(attrCoordsLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
						{
							GLClearColor(0.0, 0.0, 0.0, 1.0);

							this->CheckVSync();
						}
					}
				}
			}
		}
	}
}

// ------------------------------------------------

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
							INT margin = 5 << this->mult;

							if ((currentSub->type == SubtitlesWalk || currentSub->type == SubtitlesDescription) && !currentSub->flags)
								SetRect(&rcText, margin, margin, *(INT*)&this->virtualMode->dwHeight - margin, (LONG)this->virtualMode->dwHeight - margin);
							else
								SetRect(&rcText, margin, margin, *(INT*)&this->virtualMode->dwWidth - margin, (LONG)this->virtualMode->dwHeight - margin);

							RECT padding = { 12 << this->mult, 5 << this->mult, 12 << this->mult, 6 << this->mult };
							this->textRenderer->DrawW(currentLine->text, this->subtitlesFont->font, this->subtitlesFont->color, this->subtitlesFont->background, &rcText, &padding, TR_CENTER | TR_BOTTOM | TR_SHADOW | TR_BACKGROUND);
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
			this->textRenderer->DrawA("FPS: ", this->hFontFps, RGB(255, 255, 255), RGB(0, 0, 0), &rcText, NULL, TR_LEFT | TR_TOP | TR_SHADOW | TR_CALCULATE);

			CHAR fpsText[16];
			StrPrint(fpsText, "%.1f", this->fpsCounter->GetValue());
			SetRect(&rcText, rcText.right + 1, 5 << this->mult, (LONG)this->virtualMode->dwWidth - (10 << this->mult), (LONG)this->virtualMode->dwHeight - (5 << mult));
			this->textRenderer->DrawA(fpsText, this->hFontFps, RGB(255, 255, 0), RGB(0, 0, 0), &rcText, NULL, TR_LEFT | TR_TOP | TR_SHADOW);
		}

		this->CheckView();

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
					}
					GlobalUnlock(hMemory);

					SetClipboardData(CF_DIB, hMemory);
				}
				GlobalFree(hMemory);

				CloseClipboard();
			}
		}
	}

	return surface;
}

VOID OpenDraw::RenderFrameOld()
{
	OpenDrawSurface* surface = this->PreRender();
	if (!surface)
		return;

	SceneDataOld* renderData = (SceneDataOld*)this->sceneData;

	BOOL updateFilter = this->isStateChanged;
	if (this->isStateChanged)
	{
		this->isStateChanged = FALSE;

		if (*flags.interlaced && glCapsMultitex)
		{
			GLActiveTexture(GL_TEXTURE1);
			GLBindTexture(GL_TEXTURE_2D, renderData->scalelineId);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
			GLActiveTexture(GL_TEXTURE0);
		}
	}

	DWORD count = renderData->frameCount;
	Frame* frame = renderData->frames;
	while (count--)
	{
		GLBindTexture(GL_TEXTURE_2D, frame->id[surface->index]);

		if (updateFilter)
		{
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);
		}

		if (renderData->frameCount > 1)
		{
			DWORD* pix = (DWORD*)renderData->pixelBuffer;
			for (DWORD y = frame->rect.y; y < frame->vSize.height; ++y)
			{
				DWORD* idx = (DWORD*)this->textRenderer->dibData + y * this->virtualMode->dwWidth + frame->rect.x;
				MemoryCopy(pix, idx, frame->rect.width * sizeof(DWORD));
				pix += frame->rect.width;
			}

			GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->rect.width, frame->rect.height, GL_RGBA, GL_UNSIGNED_BYTE, renderData->pixelBuffer);
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
					GLBindTexture(GL_TEXTURE_2D, renderData->scalelineId);
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

	SwapBuffers(this->hDc);

	if (!configSingleThread)
		WaitForSingleObject(this->hDrawEvent, INFINITE);
}

VOID OpenDraw::RenderFrameNew()
{
	OpenDrawSurface* surface = this->PreRender();
	if (!surface)
		return;

	SceneDataNew* renderData = (SceneDataNew*)this->sceneData;

	if (this->isStateChanged)
	{
		this->isStateChanged = FALSE;
		UseShaderProgram(configGlFiltering == GL_LINEAR ? &renderData->shaders.bicubic : &renderData->shaders.nearest, renderData->maxTexSize);

		if (*flags.interlaced)
		{
			GLActiveTexture(GL_TEXTURE1);
			GLBindTexture(GL_TEXTURE_2D, renderData->textures.scalelineId);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, configGlFiltering);
			GLTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, configGlFiltering);

			GLActiveTexture(GL_TEXTURE0);
			GLBindTexture(GL_TEXTURE_2D, renderData->textures.normalId);
		}
	}

	GLTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->virtualMode->dwWidth, this->virtualMode->dwHeight, GL_RGBA, GL_UNSIGNED_BYTE, this->textRenderer->dibData);

	GLDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	SwapBuffers(this->hDc);

	if (!configSingleThread)
		WaitForSingleObject(this->hDrawEvent, INFINITE);
}

// ------------------------------------------------

VOID OpenDraw::RenderEndSceneOld()
{
	SceneDataOld* renderData = (SceneDataOld*)this->sceneData;
	{
		{

			{
				MemoryFree(renderData->pixelBuffer);

				// Remove
				Frame* frame = renderData->frames;
				DWORD count = renderData->frameCount;
				while (count--)
				{
					GLDeleteTextures(2, frame->id);
					++frame;
				}
			}
			MemoryFree(renderData->frames);
		}

		if (renderData->scalelineId)
			GLDeleteTextures(1, &renderData->scalelineId);
	}
	MemoryFree(renderData);
}

VOID OpenDraw::RenderEndSceneNew()
{
	SceneDataNew* renderData = (SceneDataNew*)this->sceneData;
	{
		{
			{
				{
					{
						GLBindBuffer(GL_ARRAY_BUFFER, NULL);
					}
					GLDeleteBuffers(1, &renderData->bufferName);
				}
				GLBindVertexArray(NULL);
			}
			GLDeleteVertexArrays(1, &renderData->arrayName);
		}
		GLDeleteTextures(sizeof(renderData->textures) / sizeof(GLuint), (GLuint*)&renderData->textures);

		GLUseProgram(NULL);

		ShaderProgram* shaderProgram = (ShaderProgram*)&renderData->shaders;
		DWORD count = sizeof(renderData->shaders) / sizeof(ShaderProgram);
		do
		{
			if (shaderProgram->id)
				GLDeleteProgram(shaderProgram->id);
		} while (--count);
	}
	MemoryFree(renderData);
}

// ------------------------------------------------

OpenDraw::OpenDraw()
{
	this->isStylesLoaded = FALSE;
	this->realMode = NULL;
	this->virtualMode = NULL;
	this->attachedSurface = NULL;
	this->isTakeSnapshot = FALSE;

	this->surfaceEntries = NULL;
	this->paletteEntries = NULL;
	this->clipperEntries = NULL;

	this->pRenderStartScene = NULL;
	this->pRenderEndScene = NULL;
	this->pRenderFrame = NULL;

	this->hWnd = NULL;
	this->hDraw = NULL;

	this->hDc = NULL;
	this->hRc = NULL;

	this->fpsCounter = NULL;
	this->textRenderer = NULL;

	this->hFontFps = NULL;

	this->nextSyncTime = 0.0;
	this->isFinish = TRUE;
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
		this->viewport.refresh = FALSE;
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

	devMode.dmFields |= (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL);

	if (this->realMode->dwFrequency)
	{
		devMode.dmDisplayFrequency = this->realMode->dwFrequency;
		devMode.dmFields |= DM_DISPLAYFREQUENCY;
	}

	DWORD res = ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_FULLSCREEN | CDS_TEST | CDS_RESET, NULL);
	if (res != DISP_CHANGE_SUCCESSFUL)
		return DDERR_INVALIDMODE;

	ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_FULLSCREEN | CDS_RESET, NULL);

	RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
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

		INT monWidth = GetSystemMetrics(SM_CXSCREEN);
		INT monHeight = GetSystemMetrics(SM_CYSCREEN);

		INT newWidth = (INT)MathRound(0.75f * monWidth);
		INT newHeight = (INT)MathRound(0.75f * monHeight);

		FLOAT k = (FLOAT)this->virtualMode->dwWidth / this->virtualMode->dwHeight;

		INT check = (INT)MathRound((FLOAT)newHeight * k);
		if (newWidth > check)
			newWidth = check;
		else
			newHeight = (INT)MathRound((FLOAT)newWidth / k);

		RECT* rect = &this->windowPlacement.rcNormalPosition;
		rect->left = (monWidth - newWidth) >> 1;
		rect->top = (monHeight - newHeight) >> 1;
		rect->right = rect->left + newWidth;
		rect->bottom = rect->top + newHeight;
		AdjustWindowRect(rect, WIN_STYLE, FALSE);

		this->windowPlacement.ptMinPosition.x = this->windowPlacement.ptMinPosition.y = -1;
		this->windowPlacement.ptMaxPosition.x = this->windowPlacement.ptMaxPosition.y = -1;

		this->windowPlacement.flags = NULL;
		this->windowPlacement.showCmd = SW_SHOWNORMAL;
	}

	if (!(GetWindowLong(this->hWnd, GWL_STYLE) & WS_MAXIMIZE))
	{
		SetWindowLong(this->hWnd, GWL_STYLE, WIN_STYLE);
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
			xPos = (INT)((FLOAT)(xPos - (INT)this->viewport.rectangle.x) / this->viewport.clipFactor.x);

		if (yPos < (INT)this->viewport.rectangle.y)
			yPos = 0;
		else if (yPos >= (INT)this->viewport.rectangle.y + (INT)this->viewport.rectangle.height)
			yPos = (INT)this->virtualMode->dwHeight - 1;
		else
			yPos = (INT)((FLOAT)(yPos - (INT)this->viewport.rectangle.y) / this->viewport.clipFactor.y);

		*lParam = MAKELONG(xPos, yPos);
	}
}

VOID OpenDraw::ScaleMouseOut(LPARAM* lParam)
{
	if (configDisplayWindowed || this->virtualMode != this->realMode)
	{
		INT xPos = (INT)(this->viewport.clipFactor.x * GET_X_LPARAM(*lParam));
		if (xPos < (INT)this->viewport.rectangle.x)
			xPos = 0;
		else if (xPos >= (INT)this->viewport.rectangle.x + (INT)this->viewport.rectangle.width)
			xPos = (INT)this->viewport.rectangle.x + (INT)this->viewport.rectangle.width;

		INT yPos = (INT)(this->viewport.clipFactor.y * GET_Y_LPARAM(*lParam));
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
	MemoryZero(modesList, sizeof(modesList));

	DisplayMode* mode = modesList;
	for (DWORD i = 0; i < 3; ++i)
	{
		for (DWORD j = 0; j < 3; ++j, ++mode)
		{
			mode->dwWidth = 320 << i;
			mode->dwHeight = 240 << i;
			mode->dwBPP = 8 << j;
		}
	}

	DEVMODE devMode;
	MemoryZero(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);

	DWORD bppCheck = 32;
	if (configDisplayResolution.index > 1)
		bppCheck = configDisplayResolution.bpp;
	else
	{
		if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devMode) && (devMode.dmFields & DM_BITSPERPEL))
			bppCheck = devMode.dmBitsPerPel;

		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
	}

	for (DWORD i = 0; EnumDisplaySettings(NULL, i, &devMode); ++i)
	{
		if ((devMode.dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT)) == (DM_PELSWIDTH | DM_PELSHEIGHT) &&
			devMode.dmBitsPerPel == bppCheck)
		{
			INT idx = -1;

			if (devMode.dmPelsWidth == 320 && devMode.dmPelsHeight == 240)
				idx = 0;
			else if (devMode.dmPelsWidth == 640 && devMode.dmPelsHeight == 480)
				idx = 3;
			else if (devMode.dmPelsWidth == 1280 && devMode.dmPelsHeight == 960)
				idx = 6;

			if (idx >= 0)
			{
				mode = &modesList[idx];
				for (DWORD j = 0; j < 3; ++j, ++mode)
				{
					if ((devMode.dmFields & DM_DISPLAYFREQUENCY) && mode->dwFrequency < devMode.dmDisplayFrequency)
						mode->dwFrequency = devMode.dmDisplayFrequency;

					mode->dwExists = TRUE;
				}
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
		for (DWORD i = 0; i < 3; ++i, ++mode)
		{
			if (configDisplayResolution.index <= 1 && (devMode.dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT)) == (DM_PELSWIDTH | DM_PELSHEIGHT))
			{
				mode->dwWidth = devMode.dmPelsWidth;
				mode->dwHeight = devMode.dmPelsHeight;
			}
			else
			{
				mode->dwWidth = configDisplayResolution.width;
				mode->dwHeight = configDisplayResolution.height;
			}

			mode->dwBPP = 8 << i;

			if (devMode.dmFields & DM_DISPLAYFREQUENCY)
				mode->dwFrequency = devMode.dmDisplayFrequency;

			mode->dwExists = TRUE;
		}
	}

	DDSURFACEDESC ddSurfaceDesc;
	MemoryZero(&ddSurfaceDesc, sizeof(DDSURFACEDESC));
	for (DWORD i = 0; i < 3; ++i)
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