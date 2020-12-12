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
#include "Resource.h"
#include "CommCtrl.h"
#include "Config.h"
#include "Main.h"
#include "ALib.h"

#define STR_OPENGL_AUTO "OpenGL Auto"
#define STR_OPENGL_1_1 "OpenGL 1.1"
#define STR_OPENGL_3_0 "OpenGL 3.0"

#define RECOUNT 64
Resolution* resolutionsList;
DWORD resolutionsListCount;

struct LangIndexes {
	INT voicesIndex;
	INT interfaceIndex;
	INT subtitlesIndex;
	LangFiles files;
} * langIndexes;

DWORD langIndexesCount;

CHAR* GLInit(CHAR* (*callback)())
{
	CHAR* res = NULL;

	HWND hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		WC_DRAW,
		NULL,
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		1, 1,
		NULL,
		NULL,
		hDllModule,
		NULL);

	if (hWnd)
	{
		HDC hDc = GetDC(hWnd);
		if (hDc)
		{
			GL::SetPixelFormat(hDc);

			HGLRC hRc = wglCreateContext(hDc);
			if (hRc)
			{
				if (wglMakeCurrent(hDc, hRc))
				{
					GL::CreateContextAttribs(hDc, &hRc);

					if (callback)
						res = callback();

					wglMakeCurrent(hDc, NULL);
				}

				wglDeleteContext(hRc);
			}

			ReleaseDC(hWnd, hDc);
		}

		DestroyWindow(hWnd);
	}

	return res;
}

CHAR* GetRenderer()
{
	return GLGetString ? StrDuplicate((const CHAR*)GLGetString(GL_RENDERER)) : NULL;
}

VOID __fastcall CheckGamepadConnected(HWND hDlg)
{
	BOOL devConnected = FALSE;
	if (InputGetState)
	{
		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			XINPUT_STATE xstate;
			if (InputGetState(i, &xstate) == ERROR_SUCCESS)
			{
				devConnected = TRUE;
				break;
			}
		}
	}

	HWND hCheck = GetDlgItem(hDlg, IDC_CHECK_FORCE_FEEDBACK);
	LONG dwStyle = GetWindowLong(hCheck, GWL_STYLE);
	if (devConnected)
	{
		SendDlgItemMessage(hDlg, IDC_CHECK_FORCE_FEEDBACK, BM_SETCHECK, config.other.forceFeedback ? BST_CHECKED : BST_UNCHECKED, NULL);
		SetWindowLong(hCheck, GWL_STYLE, dwStyle & ~WS_DISABLED);
	}
	else
	{
		SendDlgItemMessage(hDlg, IDC_CHECK_FORCE_FEEDBACK, BM_SETCHECK, BST_UNCHECKED, NULL);
		SetWindowLong(hCheck, GWL_STYLE, dwStyle | WS_DISABLED);
	}
}

BOOL __stdcall DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rectDlg;
		GetWindowRect(hDlg, &rectDlg);

		DWORD monWidth = GetSystemMetrics(SM_CXSCREEN);
		DWORD monHeight = GetSystemMetrics(SM_CYSCREEN);

		rectDlg.right -= rectDlg.left;
		rectDlg.bottom -= rectDlg.top;
		rectDlg.left = (monWidth - rectDlg.right) >> 1;
		rectDlg.top = (monHeight - rectDlg.bottom) >> 1;
		SetWindowPos(hDlg, NULL, rectDlg.left, rectDlg.top, rectDlg.right, rectDlg.bottom, SWP_NOZORDER);

		CHAR itemText[256];

		// OpenGL
		{
			CHAR* renderer = GLInit(GetRenderer);
			if (renderer)
			{
				StrPrint(itemText, "%s: %s", renderer, STR_OPENGL_AUTO);
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

				StrPrint(itemText, "%s: %s", renderer, STR_OPENGL_1_1);
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

				StrPrint(itemText, "%s: %s", renderer, STR_OPENGL_3_0);
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

				MemoryFree(renderer);
			}
			else
			{
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_OPENGL_AUTO);
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_OPENGL_1_1);
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_OPENGL_3_0);
			}

			DWORD val = Config::Get(CONFIG_GL, CONFIG_GL_VERSION, GL_VER_AUTO);
			switch (val)
			{
			case 1:
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_SETCURSEL, 1, NULL);
				break;
			case 3:
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_SETCURSEL, 2, NULL);
				break;
			default:
				SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_SETCURSEL, 0, NULL);
				break;
			}
		}

		// Resolution
		{
			DWORD selIdx = 1;
			DWORD val = Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, 1);
			Resolution res;
			res.width = val & 0x7FFF;
			res.height = (val >> 15) & 0x7FFF;
			res.bpp = (((val >> 30) & 0x3) + 1) << 3;

			BOOL pSupport[3] = { FALSE };

			HDC hDc = GetDC(hDlg);
			if (hDc)
			{
				PIXELFORMATDESCRIPTOR pfd;
				for (DWORD i = 0; i < 3; ++i)
				{
					GL::PreparePixelFormatDescription(&pfd);
					pfd.cColorBits = 16 + (8 * LOBYTE(i));
					pSupport[i] = ChoosePixelFormat(hDc, &pfd) != 0;
				}

				ReleaseDC(hDlg, hDc);
			}

			resolutionsList = (Resolution*)MemoryAlloc(sizeof(Resolution) * RECOUNT);
			MemoryZero(resolutionsList, sizeof(Resolution) * RECOUNT);

			DEVMODE devMode;
			MemoryZero(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			for (DWORD i = 0; EnumDisplaySettings(NULL, i, &devMode); ++i)
			{
				if ((devMode.dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL)) == (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL) && devMode.dmPelsWidth >= 320 && devMode.dmPelsHeight >= 240 && (pSupport[0] && devMode.dmBitsPerPel == 16 || pSupport[1] && devMode.dmBitsPerPel == 24 || pSupport[2] && devMode.dmBitsPerPel == 32))
				{
					Resolution* resList = resolutionsList;
					for (DWORD j = 0; j < RECOUNT; ++j, ++resList)
					{
						if (!*(DWORD*)resList)
						{
							if (!resolutionsListCount)
							{
								SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, NULL, (LPARAM) "By Game");
								SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, NULL, (LPARAM) "By Desktop");
							}

							resList->width = devMode.dmPelsWidth;
							resList->height = devMode.dmPelsHeight;
							resList->bpp = devMode.dmBitsPerPel;

							StrPrint(itemText, "Width: %d,  Height: %d,  Bit Depth: %d", resList->width, resList->height, resList->bpp);
							SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)itemText);
							++resolutionsListCount;

							if (resList->width == res.width && resList->height == res.height && resList->bpp == res.bpp)
								selIdx = resolutionsListCount + 1;

							break;
						}
						else if (resList->width == devMode.dmPelsWidth && resList->height == devMode.dmPelsHeight && resList->bpp == devMode.dmBitsPerPel)
							break;
					}
				}

				MemoryZero(&devMode, sizeof(DEVMODE));
				devMode.dmSize = sizeof(DEVMODE);
			}

			if (!resolutionsListCount)
			{
				SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, NULL, (LPARAM) "By Desktop");
				selIdx = 0;
			}
			else if (val <= 1)
				selIdx = val;

			SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_SETCURSEL, selIdx, NULL);
		}

		// Windowed
		{
			if (Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_WINDOWED, FALSE))
				SendDlgItemMessage(hDlg, IDC_CHECK_WINDOWED, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Aspect
		{
			if (Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_ASPECT, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_ASPECT, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// VSync
		{
			if (glCapsVSync)
			{
				if (Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_VSYNC, TRUE))
					SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_SETCHECK, BST_CHECKED, NULL);
			}
			else
			{
				HWND hCheck = GetDlgItem(hDlg, IDC_CHECK_VSYNC);
				SetWindowLong(hCheck, GWL_STYLE, GetWindowLong(hCheck, GWL_STYLE) | WS_DISABLED);
			}
		}

		// Filter
		{
			if (Config::Get(CONFIG_GL, CONFIG_GL_FILTERING, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// FPS limit
		{
			DWORD val = Config::Get(CONFIG_FPS, CONFIG_FPS_LIMIT, 60);
			if (!val)
				val = 60;

			SendDlgItemMessage(hDlg, IDC_EDIT_FPS_LIMIT_UPDOWND, UDM_SETRANGE, NULL, MAKELPARAM(-100000, 1));
			SendDlgItemMessage(hDlg, IDC_EDIT_FPS_LIMIT_UPDOWND, UDM_SETPOS, NULL, (LPARAM)val);
		}

		// FPS counter
		{
			if (Config::Get(CONFIG_FPS, CONFIG_FPS_COUNTER, FALSE))
				SendDlgItemMessage(hDlg, IDC_CHECK_FPS_COUNTER, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Skip intro
		{
			if (Config::Get(CONFIG_VIDEO, CONFIG_VIDEO_SKIP_INTRO, FALSE))
				SendDlgItemMessage(hDlg, IDC_CHECK_SKIP, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Smoother movies
		{
			if (Config::Get(CONFIG_VIDEO, CONFIG_VIDEO_SMOOTHER, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_SMOOTH, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Static camera
		{
			if (Config::Get(CONFIG_CAMERA, CONFIG_CAMERA_STATIC, FALSE))
				SendDlgItemMessage(hDlg, IDC_CHECK_STATIC_CAMERA, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Zommed camera
		{
			if (Config::Get(CONFIG_CAMERA, CONFIG_CAMERA_ZOOMED, FALSE))
				SendDlgItemMessage(hDlg, IDC_CHECK_ZOOMED_CAMERA, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// 3D audio
		{
			if (AL::Load())
			{
				if (Config::Get(CONFIG_OTHER, CONFIG_OTHER_3D_SOUND, TRUE))
					SendDlgItemMessage(hDlg, IDC_CHECK_3D_SOUND, BM_SETCHECK, BST_CHECKED, NULL);
			}
			else
			{
				HWND hCheck = GetDlgItem(hDlg, IDC_CHECK_3D_SOUND);
				SetWindowLong(hCheck, GWL_STYLE, GetWindowLong(hCheck, GWL_STYLE) | WS_DISABLED);
			}
		}

		// Force feedback
		{
			config.other.forceFeedback = Config::Get(CONFIG_OTHER, CONFIG_OTHER_FORCE_FEEDBACK, TRUE);
			CheckGamepadConnected(hDlg);
		}

		// Languages
		{
			CHAR iniFile[MAX_PATH];
			StrPrint(iniFile, "%s\\LOCALE.INI", kainDirPath);

			SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_ADDSTRING, 0, (LPARAM) "Off");

			langIndexesCount = GetPrivateProfileInt("LOCALE", "Languages", 0, iniFile);
			SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_SETCURSEL, 0, NULL);

			if (langIndexesCount)
			{
				langIndexes = (LangIndexes*)MemoryAlloc(sizeof(LangIndexes) * langIndexesCount);
				MemoryZero(langIndexes, sizeof(LangIndexes) * langIndexesCount);

				config.language.voices = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_VOICES, 0);
				config.language.display = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_INTERFACE, 0);
				config.language.subtitles = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_SUBTITLES, 0);

				LangIndexes* currLangIndex = langIndexes;

				CHAR langKey[16];
				CHAR langName[64];
				for (INT i = 0, vIdx = 0, iIdx = 0, sIdx = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					StrPrint(langKey, "LANG_%d", i);
					if (GetPrivateProfileString(langKey, "Name", "", langName, sizeof(langName) - 1, iniFile))
					{
						CHAR langFileName[MAX_PATH];
						if (GetPrivateProfileString(langKey, "AudioFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
							StrPrint(currLangIndex->files.audioFile, "%s\\%s", kainJamPath, langFileName);

						if (GetPrivateProfileString(langKey, "VoicesFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
						{
							StrPrint(currLangIndex->files.voicesFile, "%s\\%s", kainDirPath, langFileName);

							FILE* hFile = FileOpen(currLangIndex->files.voicesFile, "rb");
							if (hFile)
							{
								FileClose(hFile);

								SendDlgItemMessage(hDlg, IDC_COMBO_LANG_VOICES, CB_ADDSTRING, 0, (LPARAM)langName);
								if (i == config.language.voices)
									SendDlgItemMessage(hDlg, IDC_COMBO_LANG_VOICES, CB_SETCURSEL, vIdx, NULL);

								currLangIndex->voicesIndex = ++vIdx;
							}
						}

						if (GetPrivateProfileString(langKey, "InterfaceFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
						{
							StrPrint(currLangIndex->files.interfaceFile, "%s\\%s", kainDirPath, langFileName);

							FILE* hFile = FileOpen(currLangIndex->files.interfaceFile, "rb");
							if (hFile)
							{
								FileClose(hFile);

								SendDlgItemMessage(hDlg, IDC_COMBO_LANG_INTERFACE, CB_ADDSTRING, 0, (LPARAM)langName);
								if (i == config.language.display)
									SendDlgItemMessage(hDlg, IDC_COMBO_LANG_INTERFACE, CB_SETCURSEL, iIdx, NULL);

								currLangIndex->interfaceIndex = ++iIdx;
							}
						}

						if (GetPrivateProfileString(langKey, "SubtitlesFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
						{
							StrPrint(currLangIndex->files.subtitlesFile, "%s\\%s", kainDirPath, langFileName);

							FILE* hFile = FileOpen(currLangIndex->files.subtitlesFile, "rb");
							if (hFile)
							{
								FileClose(hFile);

								SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_ADDSTRING, 0, (LPARAM)langName);
								if (i == config.language.subtitles)
									SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_SETCURSEL, sIdx + 1, NULL);

								currLangIndex->subtitlesIndex = ++sIdx;
							}
						}
					}
				}
			}
		}

		return TRUE;
	}

	case WM_DEVICECHANGE:
	{
		CheckGamepadConnected(hDlg);
		return DefWindowProc(hDlg, uMsg, wParam, lParam);
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			// OpenGL
			{
				switch (SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_GETCURSEL, NULL, NULL))
				{
				case 1:
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, GL_VER_1))
						config.gl.version = GL_VER_1;
					break;
				case 2:
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, GL_VER_3))
						config.gl.version = GL_VER_3;
					break;
				default:
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, GL_VER_AUTO))
						config.gl.version = GL_VER_AUTO;
					break;
				}
			}

			// Resolution
			{
				if (resolutionsListCount)
				{
					DWORD val = SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_GETCURSEL, NULL, NULL);
					if (val < RECOUNT + 2)
					{
						switch (val)
						{
						case 0:
						case 1:
							if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, val))
								config.display.resolution.index = val;
							break;
						default:
						{
							Resolution* res = &resolutionsList[val - 2];

							val = (res->width & 0x7FFF) | ((res->height & 0x7FFF) << 15) | (((res->bpp >> 3) - 1) << 30);
							if (val)
							{
								if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, val))
									config.display.resolution = *res;
							}
							else
							{
								if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, 0))
									config.display.resolution.index = 0;
							}
							break;
						}
						}
					}
				}
				else if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, 1))
					config.display.resolution.index = 1;

				MemoryFree(resolutionsList);
			}

			// Windowed
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_WINDOWED, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_WINDOWED, val))
					config.display.windowed = val;
			}

			// Aspect
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_ASPECT, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_ASPECT, val))
					config.display.aspect = val;
			}

			// VSync
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_VSYNC, val))
					config.display.vSync = val;
			}

			// Filter
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_GL, CONFIG_GL_FILTERING, val))
					config.gl.filtering = val ? GL_LINEAR : GL_NEAREST;
			}

			// FPS limit
			{
				DWORD val = SendDlgItemMessage(hDlg, IDC_EDIT_FPS_LIMIT_UPDOWND, UDM_GETPOS, NULL, NULL);
				if (Config::Set(CONFIG_FPS, CONFIG_FPS_LIMIT, val))
					config.fps.limit = 1.0f / val;
			}

			// FPS counter
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_FPS_COUNTER, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_FPS, CONFIG_FPS_COUNTER, val))
					config.fps.counter = val;
			}

			// Skip intro
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_SKIP, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_VIDEO, CONFIG_VIDEO_SKIP_INTRO, val))
					config.video.skipIntro = val;
			}

			// Smoother movies
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_SMOOTH, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_VIDEO, CONFIG_VIDEO_SMOOTHER, val))
					config.video.smoother = val;
			}

			// Languages
			{
				config.language.voices = 0;
				INT val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_VOICES, CB_GETCURSEL, NULL, NULL) + 1;
				LangIndexes* currLangIndex = langIndexes;
				for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					if (currLangIndex->voicesIndex == val)
					{
						config.language.voices = i;
						StrCopy(langFiles.audioFile, currLangIndex->files.audioFile);
						StrCopy(langFiles.voicesFile, currLangIndex->files.voicesFile);
						break;
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_VOICES, config.language.voices);

				config.language.display = 0;
				val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_INTERFACE, CB_GETCURSEL, NULL, NULL) + 1;
				currLangIndex = langIndexes;
				for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					if (currLangIndex->interfaceIndex == val)
					{
						config.language.display = i;
						StrCopy(langFiles.interfaceFile, currLangIndex->files.interfaceFile);
						break;
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_INTERFACE, config.language.display);

				config.language.subtitles = -1;
				val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_GETCURSEL, NULL, NULL);
				if (val)
				{
					currLangIndex = langIndexes;
					for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
					{
						if (currLangIndex->subtitlesIndex == val)
						{
							config.language.subtitles = i;
							StrCopy(langFiles.subtitlesFile, currLangIndex->files.subtitlesFile);
							break;
						}
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_SUBTITLES, config.language.subtitles);

				if (langIndexes)
					MemoryFree(langIndexes);
			}

			// Static camera
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_STATIC_CAMERA, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_CAMERA, CONFIG_CAMERA_STATIC, val))
					*config.camera.isStatic = val;
			}

			// Zommed camera
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_ZOOMED_CAMERA, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_CAMERA, CONFIG_CAMERA_ZOOMED, val))
					config.camera.isZoomed = val;
			}

			// 3D audio
			{
				if (!(GetWindowLong(GetDlgItem(hDlg, IDC_CHECK_3D_SOUND), GWL_STYLE) & WS_DISABLED))
				{
					BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_3D_SOUND, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
					if (Config::Set(CONFIG_OTHER, CONFIG_OTHER_3D_SOUND, val))
						config.other.sound3d = val;
				}

				if (!config.other.sound3d)
					AL::Free();
			}

			// Force feedback
			{
				if (!(GetWindowLong(GetDlgItem(hDlg, IDC_CHECK_FORCE_FEEDBACK), GWL_STYLE) & WS_DISABLED))
				{
					BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_FORCE_FEEDBACK, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
					if (Config::Set(CONFIG_OTHER, CONFIG_OTHER_FORCE_FEEDBACK, val))
						config.other.forceFeedback = val;
				}
			}

			EndDialog(hDlg, LOWORD(wParam));
			break;
		}

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, NULL, NULL);
			break;

		default:
			break;
		}

		return FALSE;
	}

	case WM_CLOSE:
		DefWindowProc(hDlg, uMsg, wParam, lParam);
		return FALSE;

	default:
		return FALSE;
	}
}

INT __stdcall MessageBoxHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	INT res;

	if (hActCtx && hActCtx != INVALID_HANDLE_VALUE)
	{
		ULONG_PTR cookie = NULL;
		if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
			cookie = NULL;

		res = MessageBox(hWnd, lpText, lpCaption, uType);

		if (cookie)
			DeactivateActCtxC(0, cookie);
	}
	else
		res = MessageBox(hWnd, lpText, lpCaption, uType);

	return res;
}

HICON __stdcall LoadIconHook(HINSTANCE hInstance, LPCSTR lpIconName)
{
	return LoadIcon(GetModuleHandle(NULL), "SETUP_ICON");
}

ATOM __stdcall RegisterClassHook(WNDCLASSA* lpWndClass)
{
	lpWndClass->hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	return RegisterClass(lpWndClass);
}

HWND __stdcall CreateWindowExHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT X, INT Y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	INT monWidth = GetSystemMetrics(SM_CXSCREEN);
	INT monHeight = GetSystemMetrics(SM_CYSCREEN);

	if (config.display.windowed)
	{
		dwStyle = WIN_STYLE;

		nWidth = (INT)MathRound(0.75f * monWidth);
		nHeight = (INT)MathRound(0.75f * monHeight);

		FLOAT k = 4.0f / 3.0f;

		INT check = (INT)MathRound((FLOAT)nHeight * k);
		if (nWidth > check)
			nWidth = check;
		else
			nHeight = (INT)MathRound((FLOAT)nWidth / k);
	}
	else
	{
		dwStyle = FS_STYLE | WS_CAPTION;

		nWidth = monWidth;
		nHeight = monHeight;
	}

	X = (monWidth - nWidth) >> 1;
	Y = (monHeight - nHeight) >> 1;

	RECT rect = { X, Y, X + nWidth, Y + nHeight };
	AdjustWindowRect(&rect, dwStyle, FALSE);

	X = rect.left;
	Y = rect.top;
	nWidth = rect.right - rect.left;
	nHeight = rect.bottom - rect.top;

	Hooks::hMainWnd = CreateWindowEx(WS_EX_APPWINDOW, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (!config.display.windowed)
	{
		dwStyle = FS_STYLE;
		SetWindowLong(Hooks::hMainWnd, GWL_STYLE, dwStyle);

		RECT rect = { 0, 0, monWidth, monHeight };
		AdjustWindowRect(&rect, dwStyle, FALSE);

		SetWindowPos(Hooks::hMainWnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
	}

	return Hooks::hMainWnd;
}

HWND __stdcall GetActiveWindowHook()
{
	return Hooks::hMainWnd;
}

namespace Hooks
{
#pragma optimize("s", on)
	VOID Patch_Window(HOOKER hooker)
	{
		BOOL noWindow = FALSE;
		CHAR* line = GetCommandLine();
		do
		{
			line = StrChar(line, ' ');
			if (!line || !*(++line))
				break;

			if (StrStr(line, "-noconfig") == line)
			{
				noWindow = TRUE;
				break;
			}
		} while (TRUE);

		if (!noWindow)
		{
			// Load Common Controls classes for UpDown
			INITCOMMONCONTROLSEX cc = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES };
			InitCommonControlsEx(&cc);

			INT_PTR res;
			ULONG_PTR cookie = NULL;
			if (hActCtx && hActCtx != INVALID_HANDLE_VALUE && !ActivateActCtxC(hActCtx, &cookie))
				cookie = NULL;

			res = DialogBoxParam(hDllModule, MAKEINTRESOURCE(IDD_DIALOGBAR), NULL, DlgProc, NULL);

			if (cookie)
				DeactivateActCtxC(0, cookie);

			if (res <= 0)
				Exit(EXIT_FAILURE);
		}
		else
		{
			// OpenGL
			config.gl.version = Config::Get(CONFIG_GL, CONFIG_GL_VERSION, GL_VER_AUTO);
			if (config.gl.version != GL_VER_1 && config.gl.version != GL_VER_3)
				config.gl.version = GL_VER_AUTO;

			// Resolution
			DWORD val = Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, 1);
			if (val > 1)
			{
				config.display.resolution.width = val & 0x7FFF;
				config.display.resolution.height = (val >> 15) & 0x7FFF;
				config.display.resolution.bpp = (((val >> 30) & 0x3) + 1) << 3;

				BOOL found = FALSE;
				DEVMODE devMode;
				MemoryZero(&devMode, sizeof(DEVMODE));
				devMode.dmSize = sizeof(DEVMODE);
				for (DWORD i = 0; EnumDisplaySettings(NULL, i, &devMode); ++i)
				{
					if ((devMode.dmFields & (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL)) == (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL) && devMode.dmPelsWidth == config.display.resolution.width && devMode.dmPelsHeight == config.display.resolution.height && devMode.dmBitsPerPel == config.display.resolution.bpp)
					{
						found = TRUE;
						break;
					}

					MemoryZero(&devMode, sizeof(DEVMODE));
					devMode.dmSize = sizeof(DEVMODE);
				}

				if (!found)
				{
					config.display.resolution.width = 0;
					config.display.resolution.height = 0;
					config.display.resolution.index = 1;
				}
			}
			else
				config.display.resolution.index = val;

			// Windowed
			config.display.windowed = Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_WINDOWED, FALSE);

			// Aspect
			config.display.aspect = Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_ASPECT, TRUE);

			// VSync
			{
				config.display.vSync = Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_VSYNC, TRUE);
				if (config.display.vSync)
				{
					GLInit(NULL);
					if (!glCapsVSync)
						config.display.vSync = FALSE;
				}
			}

			// Filter
			config.gl.filtering = Config::Get(CONFIG_GL, CONFIG_GL_FILTERING, TRUE) ? GL_LINEAR : GL_NEAREST;

			// FPS limit
			config.fps.limit = 1.0f / Config::Get(CONFIG_FPS, CONFIG_FPS_LIMIT, 60);

			// FPS counter
			config.fps.counter = Config::Get(CONFIG_FPS, CONFIG_FPS_COUNTER, FALSE);

			// Skip intro
			config.video.skipIntro = Config::Get(CONFIG_VIDEO, CONFIG_VIDEO_SKIP_INTRO, FALSE);

			// Smoother movies
			config.video.smoother = Config::Get(CONFIG_VIDEO, CONFIG_VIDEO_SMOOTHER, TRUE);

			// Static camera
			*config.camera.isStatic = Config::Get(CONFIG_CAMERA, CONFIG_CAMERA_STATIC, FALSE);

			// Zommed camera
			config.camera.isZoomed = Config::Get(CONFIG_CAMERA, CONFIG_CAMERA_ZOOMED, TRUE);

			// 3D audio
			config.other.sound3d = Config::Get(CONFIG_OTHER, CONFIG_OTHER_3D_SOUND, TRUE);
			if (config.other.sound3d && !AL::Load())
				config.other.sound3d = FALSE;

			// Force feedback
			config.other.forceFeedback = Config::Get(CONFIG_OTHER, CONFIG_OTHER_FORCE_FEEDBACK, TRUE);

			// Languages
			{
				CHAR iniFile[MAX_PATH];
				StrPrint(iniFile, "%s\\LOCALE.INI", kainDirPath);

				langIndexesCount = GetPrivateProfileInt("LOCALE", "Languages", 0, iniFile);
				if (langIndexesCount)
				{
					config.language.voices = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_VOICES, 0);
					config.language.display = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_INTERFACE, 0);
					config.language.subtitles = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_SUBTITLES, 0);

					CHAR langKey[16];
					CHAR langName[64];
					CHAR langFile[MAX_PATH];
					for (INT i = 0; i < (INT)langIndexesCount; ++i)
					{
						if (i == config.language.voices || i == config.language.display || i == config.language.subtitles)
						{
							StrPrint(langKey, "LANG_%d", i);
							if (GetPrivateProfileString(langKey, "Name", "", langName, sizeof(langName) - 1, iniFile))
							{
								CHAR langFileName[MAX_PATH];
								if (i == config.language.voices && GetPrivateProfileString(langKey, "AudioFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
								{
									StrPrint(langFile, "%s\\%s", kainJamPath, langFileName);

									FILE* hFile = FileOpen(langFile, "rb");
									if (hFile)
									{
										FileClose(hFile);
										StrCopy(langFiles.audioFile, langFile);
									}
								}

								if (i == config.language.voices && GetPrivateProfileString(langKey, "VoicesFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
								{
									StrPrint(langFile, "%s\\%s", kainDirPath, langFileName);

									FILE* hFile = FileOpen(langFile, "rb");
									if (hFile)
									{
										FileClose(hFile);
										StrCopy(langFiles.voicesFile, langFile);
									}
								}

								if (i == config.language.display && GetPrivateProfileString(langKey, "InterfaceFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
								{
									StrPrint(langFile, "%s\\%s", kainDirPath, langFileName);

									FILE* hFile = FileOpen(langFile, "rb");
									if (hFile)
									{
										FileClose(hFile);
										StrCopy(langFiles.interfaceFile, langFile);
									}
								}

								if (i == config.language.subtitles && GetPrivateProfileString(langKey, "SubtitlesFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
								{
									StrPrint(langFile, "%s\\%s", kainDirPath, langFileName);

									FILE* hFile = FileOpen(langFile, "rb");
									if (hFile)
									{
										FileClose(hFile);
										StrCopy(langFiles.subtitlesFile, langFile);
									}
								}
							}
						}
					}
				}
			}
		}

		// xBox gamepad
		config.other.xboxConfig = Config::Get(CONFIG_OTHER, CONFIG_OTHER_XBOX_CONFIG, TRUE);

		PatchImportByName(hooker, "MessageBoxA", MessageBoxHook);
		PatchImportByName(hooker, "LoadIconA", LoadIconHook);
		PatchImportByName(hooker, "RegisterClassA", RegisterClassHook);
		PatchImportByName(hooker, "CreateWindowExA", CreateWindowExHook);
		PatchImportByName(hooker, "GetActiveWindow", GetActiveWindowHook);
		PatchImportByName(hooker, "GetActiveWindow", GetActiveWindowHook); // second time

		HMODULE hLibrary = LoadLibrary("NTDLL.dll");
		if (hLibrary)
		{
			if (GetProcAddress(hLibrary, "wine_get_version"))
				config.single.window = TRUE;
			FreeLibrary(hLibrary);
		}

		config.single.thread = TRUE;
		DWORD processMask, systemMask;
		HANDLE process = GetCurrentProcess();
		if (GetProcessAffinityMask(process, &processMask, &systemMask))
		{
			if (processMask != systemMask && SetProcessAffinityMask(process, systemMask))
				GetProcessAffinityMask(process, &processMask, &systemMask);

			BOOL isSingle = FALSE;
			DWORD count = sizeof(DWORD) << 3;
			do
			{
				if (processMask & 1)
				{
					if (isSingle)
					{
						config.single.thread = FALSE;
						break;
					}
					else
						isSingle = TRUE;
				}

				processMask >>= 1;
			} while (--count);
		}

		DEVMODE devMode;
		MemoryZero(&devMode, sizeof(DEVMODE));
		devMode.dmSize = sizeof(DEVMODE);
		config.fps.sync = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) && (devMode.dmFields & DM_DISPLAYFREQUENCY) ? 1.0f / devMode.dmDisplayFrequency : 0.0;
	}
#pragma optimize("", on)
}