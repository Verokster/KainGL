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
#include "Hooks.h"
#include "Resource.h"
#include "CommCtrl.h"
#include "Config.h"
#include "Main.h"
#include "ALib.h"

#define WIN_STYLE DS_MODALFRAME | DS_CENTER | DS_CENTERMOUSE | WS_POPUP | WS_CAPTION | WS_SYSMENU

#define STR_AUTO "Auto"
#define STR_OPENGL_1_1 "OpenGL 1.1"
#define STR_OPENGL_3_0 "OpenGL 3.0"
#define LEGACY_OF_KAIN "Legacy of Kain: Blood Omen"

#define RECOUNT 64
Resolution resolutionsList[RECOUNT];

struct LangIndexes
{
	INT voicesIndex;
	INT interfaceIndex;
	INT subtitlesIndex;
	LangFiles files;
} *langIndexes;

DWORD langIndexesCount;

VOID GetMonitorRect(HWND hWnd, RECT* rectMon)
{
	MONITORINFO mi = { sizeof(mi) };
	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMon && GetMonitorInfo(hMon, &mi))
	{
		*rectMon = mi.rcWork;
		return;
	}

	HWND hWndDesctop = GetDesktopWindow();
	GetWindowRect(hWndDesctop, rectMon);
}

BOOL __stdcall DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetWindowText(hDlg, LEGACY_OF_KAIN);

		RECT rectDlg;
		GetWindowRect(hDlg, &rectDlg);

		RECT rectMon;
		GetMonitorRect(hDlg, &rectMon);

		rectDlg.right -= rectDlg.left;
		rectDlg.bottom -= rectDlg.top;
		rectDlg.left = rectMon.left + ((rectMon.right - rectMon.left) - rectDlg.right) / 2;
		rectDlg.top = rectMon.top + ((rectMon.bottom - rectMon.top) - rectDlg.bottom) / 2;

		SetWindowPos(hDlg, NULL, rectDlg.left, (rectDlg.top >= rectMon.top ? rectDlg.top : rectMon.top), rectDlg.right, rectDlg.bottom, SWP_NOZORDER);

		DWORD i;
		CHAR itemText[256];
		DWORD selIndex = 0;

		// OpenGL
		{
			DISPLAY_DEVICE device;
			MemoryZero(&device, sizeof(DISPLAY_DEVICE));
			device.cb = sizeof(DISPLAY_DEVICE);
			for (i = 0; EnumDisplayDevices(NULL, i, &device, NULL); ++i)
			{
				if (device.StateFlags & (DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
				{
					if (*device.DeviceString)
					{
						StrPrint(itemText, "%s: %s", device.DeviceString, STR_AUTO);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

						StrPrint(itemText, "%s: %s", device.DeviceString, STR_OPENGL_1_1);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

						StrPrint(itemText, "%s: %s", device.DeviceString, STR_OPENGL_3_0);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);
					}
					else
					{
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_AUTO);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_OPENGL_1_1);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)STR_OPENGL_3_0);
					}
					break;
				}

				MemoryZero(&device, sizeof(DISPLAY_DEVICE));
				device.cb = sizeof(DISPLAY_DEVICE);
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

			SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, NULL, (LPARAM)"By Game");
			SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, NULL, (LPARAM)"By Desktop");

			DEVMODE devMode;
			MemoryZero(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);

			BOOL pSupport[3] = { FALSE };

			HDC hDc = GetDC(hDlg);
			if (hDc)
			{
				PIXELFORMATDESCRIPTOR pfd;
				for (i = 0; i < 3; ++i)
				{
					GL::PreparePixelFormatDescription(&pfd);
					pfd.cColorBits = 16 + (8 * LOBYTE(i));
					pSupport[i] = ChoosePixelFormat(hDc, &pfd) != 0;
				}
			}
			ReleaseDC(hDlg, hDc);

			DWORD idx = 1;
			for (i = 0; EnumDisplaySettings(NULL, i, &devMode); ++i)
			{
				if (devMode.dmPelsWidth >= 320 && devMode.dmPelsHeight >= 240 && (
					pSupport[0] && devMode.dmBitsPerPel == 16 ||
					pSupport[1] && devMode.dmBitsPerPel == 24 ||
					pSupport[2] && devMode.dmBitsPerPel == 32))
				{
					Resolution* resList = resolutionsList;
					for (DWORD j = 0; j < RECOUNT; ++j, ++resList)
					{
						if (!*(DWORD*)resList)
						{
							resList->width = devMode.dmPelsWidth;
							resList->height = devMode.dmPelsHeight;
							resList->bpp = devMode.dmBitsPerPel;

							StrPrint(itemText, "Width: %d,  Height: %d,  Bit Depth: %d", resList->width, resList->height, resList->bpp);
							SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)itemText);
							++idx;

							if (resList->width == res.width && resList->height == res.height && resList->bpp == res.bpp)
								selIdx = idx;

							break;
						}
						else if (resList->width == devMode.dmPelsWidth && resList->height == devMode.dmPelsHeight && resList->bpp == devMode.dmBitsPerPel)
							break;
					}
				}

				MemoryZero(&devMode, sizeof(DEVMODE));
				devMode.dmSize = sizeof(DEVMODE);
			}

			if (val == 0 || val == 1)
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
			if (Config::Get(CONFIG_DISPLAY, CONFIG_DISPLAY_VSYNC, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// Filter
		{
			if (Config::Get(CONFIG_GL, CONFIG_GL_FILTERING, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_SETCHECK, BST_CHECKED, NULL);
		}

		// FPS limit
		{
			DWORD val = Config::Get(CONFIG_FPS, CONFIG_FPS_LIMIT, 50);
			if (!val)
				val = 50;

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
			if (Config::Get(CONFIG_OTHER, CONFIG_OTHER_STATIC_CAMERA, TRUE))
				SendDlgItemMessage(hDlg, IDC_CHECK_STATIC_CAMERA, BM_SETCHECK, BST_CHECKED, NULL);
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

		{
			CHAR iniFile[MAX_PATH];
			StrPrint(iniFile, "%s\\LOCALE.INI", kainDirPath);

			SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_ADDSTRING, 0, (LPARAM)"Off");

			langIndexesCount = GetPrivateProfileInt("LOCALE", "Languages", 0, iniFile);
			SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_SETCURSEL, 0, NULL);

			if (langIndexesCount)
			{
				langIndexes = (LangIndexes*)MemoryAlloc(sizeof(LangIndexes) * langIndexesCount);
				MemoryZero(langIndexes, sizeof(LangIndexes) * langIndexesCount);

				configLangVoices = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_VOICES, 0);
				configLangInterface = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_INTERFACE, 0);
				configLangSubtitles = Config::Get(CONFIG_LANGUAGE, CONFIG_LANGUAGE_SUBTITLES, 0);

				LangIndexes* currLangIndex = langIndexes;

				CHAR langKey[16];
				CHAR langName[64];
				for (INT i = 0, vIdx = 0, iIdx = 0, sIdx = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					StrPrint(langKey, "LANG_%d", i);
					if (GetPrivateProfileString(langKey, "Name", "", langName, sizeof(langName) - 1, iniFile))
					{
						CHAR langFileName[MAX_PATH];
						if (GetPrivateProfileString(langKey, "VoicesFile", "", langFileName, sizeof(langFileName) - 1, iniFile))
						{
							StrPrint(currLangIndex->files.voicesFile, "%s\\%s", kainDirPath, langFileName);

							FILE* hFile = FileOpen(currLangIndex->files.voicesFile, "rb");
							if (hFile)
							{
								FileClose(hFile);

								SendDlgItemMessage(hDlg, IDC_COMBO_LANG_VOICES, CB_ADDSTRING, 0, (LPARAM)langName);
								if (i == configLangVoices)
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
								if (i == configLangInterface)
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
								if (i == configLangSubtitles)
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
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, 1))
						configGlVersion = GL_VER_1;
					break;
				case 2:
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, 3))
						configGlVersion = GL_VER_3;
					break;
				default:
					if (Config::Set(CONFIG_GL, CONFIG_GL_VERSION, 0))
						configGlVersion = GL_VER_AUTO;
					break;
				}
			}

			// Resolution
			{
				DWORD val = SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_GETCURSEL, NULL, NULL);
				if (val < RECOUNT + 2)
				{
					switch (val)
					{
					case 0:
					case 1:
						if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, val))
							configDisplayResolution.index = val;
						break;
					default:
					{
						Resolution *res = &resolutionsList[val - 2];

						DWORD val = (res->width & 0x7FFF) | ((res->height & 0x7FFF) << 15) | (((res->bpp >> 3) - 1) << 30);
						if (val)
						{
							if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, val))
								configDisplayResolution = *res;
						}
						else
						{
							if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_RESOLUTION, 0))
								configDisplayResolution.index = 0;
						}
						break;
					}
					}
				}
			}

			// Windowed
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_WINDOWED, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_WINDOWED, val))
					configDisplayWindowed = val;
			}

			// Aspect
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_ASPECT, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_ASPECT, val))
					configDisplayAspect = val;
			}

			// VSync
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_DISPLAY, CONFIG_DISPLAY_VSYNC, val))
					configDisplayVSync = val;
			}

			// Filter
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_GL, CONFIG_GL_FILTERING, val))
					configGlFiltering = val ? GL_LINEAR : GL_NEAREST;
			}

			// FPS limit
			{
				DWORD val = SendDlgItemMessage(hDlg, IDC_EDIT_FPS_LIMIT_UPDOWND, UDM_GETPOS, NULL, NULL);
				if (Config::Set(CONFIG_FPS, CONFIG_FPS_LIMIT, val))
					configFpsLimit = FLOAT(1000.0 / val);
			}

			// FPS counter
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_FPS_COUNTER, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_FPS, CONFIG_FPS_COUNTER, val))
					configFpsCounter = val;
			}

			// Skip intro
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_SKIP, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_VIDEO, CONFIG_VIDEO_SKIP_INTRO, val))
					configVideoSkipIntro = val;
			}

			// Smoother movies
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_SMOOTH, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_VIDEO, CONFIG_VIDEO_SMOOTHER, val))
					configVideoSmoother = val;
			}

			// Languages
			{
				configLangVoices = 0;
				INT val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_VOICES, CB_GETCURSEL, NULL, NULL) + 1;
				LangIndexes* currLangIndex = langIndexes;
				for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					if (currLangIndex->voicesIndex == val)
					{
						configLangVoices = i;
						StrCopy(langFiles.voicesFile, currLangIndex->files.voicesFile);
						break;
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_VOICES, configLangVoices);

				configLangInterface = 0;
				val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_INTERFACE, CB_GETCURSEL, NULL, NULL) + 1;
				currLangIndex = langIndexes;
				for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
				{
					if (currLangIndex->interfaceIndex == val)
					{
						configLangInterface = i;
						StrCopy(langFiles.interfaceFile, currLangIndex->files.interfaceFile);
						break;
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_INTERFACE, configLangInterface);

				configLangSubtitles = -1;
				val = SendDlgItemMessage(hDlg, IDC_COMBO_LANG_SUBTITLES, CB_GETCURSEL, NULL, NULL);
				if (val)
				{
					currLangIndex = langIndexes;
					for (INT i = 0; i < (INT)langIndexesCount; ++i, ++currLangIndex)
					{
						if (currLangIndex->subtitlesIndex == val)
						{
							configLangSubtitles = i;
							StrCopy(langFiles.subtitlesFile, currLangIndex->files.subtitlesFile);
							break;
						}
					}
				}
				Config::Set(CONFIG_LANGUAGE, CONFIG_LANGUAGE_SUBTITLES, configLangSubtitles);

				if (langIndexes)
					MemoryFree(langIndexes);
			}

			// Static camera
			{
				BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_STATIC_CAMERA, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
				if (Config::Set(CONFIG_OTHER, CONFIG_OTHER_STATIC_CAMERA, val))
					*configOtherStaticCamera = val;
			}

			// 3D audio
			{
				if (!(GetWindowLong(GetDlgItem(hDlg, IDC_CHECK_3D_SOUND), GWL_STYLE) & WS_DISABLED))
				{
					BOOL val = SendDlgItemMessage(hDlg, IDC_CHECK_3D_SOUND, BM_GETCHECK, NULL, NULL) == BST_CHECKED;
					if (Config::Set(CONFIG_OTHER, CONFIG_OTHER_3D_SOUND, val))
						configOther3DSound = val;
				}

				if (!configOther3DSound)
				{
					AL::Free();
					LoadDSound();
				}
			}

			configOtherXboxConfig = Config::Get(CONFIG_OTHER, CONFIG_OTHER_XBOX_CONFIG, TRUE);

			Hooks::Patch_Video();
			Hooks::Patch_Subtitles();

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

VOID __stdcall OpenWindow()
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

DWORD back_0045F504 = 0x0045F504;
VOID __declspec(naked) hook_0045F4F8()
{
	_asm
	{
		PUSH EBX
		PUSH ESI
		PUSH EDI
		PUSH EBP
		MOV EBP, ESP
		SUB ESP, 0x30
		CALL OpenWindow
		JMP back_0045F504
	}
}

HWND __stdcall GetActiveWindowHook()
{
	HWND hWnd = GetActiveWindow();

	OpenDraw* ddraw = Main::FindDrawByWindow(hWnd);
	if (ddraw)
		hWnd = ddraw->hWnd;

	return hWnd;
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

namespace Hooks
{
	VOID Patch_Window()
	{
		// Window text & icon
		PatchDWord(0x0045F5DA + 1, WS_POPUP | WS_SYSMENU | WS_CAPTION);
		PatchDWord(0x0045F546 + 1, (DWORD)"SETUP_ICON");
		PatchWord(0x0045F5DF, 0x6890);
		PatchDWord(0x0045F5DF + 2, (DWORD)LEGACY_OF_KAIN);
		PatchWord(0x0045F54B, 0x9050); // PUSH EAX

		PatchHook(0x0045F4F8, hook_0045F4F8);
		back_0045F504 += baseAddress;

		PatchFunction("GetActiveWindow", GetActiveWindowHook);
		PatchFunction("MessageBoxA", MessageBoxHook);
	}
}