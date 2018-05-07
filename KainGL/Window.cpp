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

#define WIN_STYLE DS_MODALFRAME | DS_CENTER | DS_CENTERMOUSE | WS_POPUP | WS_CAPTION | WS_SYSMENU

#define STR_AUTO "Auto"
#define STR_OPENGL_1_1 "OpenGL 1.1"
#define STR_OPENGL_3_0 "OpenGL 3.0"
#define LEGACY_OF_KAIN "Legacy of Kain: Blood Omen"

#define RECOUNT 64
Resolution resolutionsList[RECOUNT];

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

BOOL __stdcall DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
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
			DISPLAY_DEVICE device = { NULL };
			device.cb = sizeof(DISPLAY_DEVICE);
			for (i = 0; EnumDisplayDevices(NULL, i, &device, NULL); ++i)
			{
				if (device.StateFlags & (DISPLAY_DEVICE_PRIMARY_DEVICE | DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
				{
					if (*device.DeviceString)
					{
						sprintf(itemText, "%s: %s", device.DeviceString, STR_AUTO);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

						sprintf(itemText, "%s: %s", device.DeviceString, STR_OPENGL_1_1);
						SendDlgItemMessage(hDlg, IDC_COMBO_DRIVER, CB_ADDSTRING, NULL, (LPARAM)itemText);

						sprintf(itemText, "%s: %s", device.DeviceString, STR_OPENGL_3_0);
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

				memset(&device, NULL, sizeof(DISPLAY_DEVICE));
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

			DEVMODE devMode = { NULL };
			devMode.dmSize = sizeof(DEVMODE);

			BOOL pSupport[3] = { FALSE };

			HDC hDc = GetDC(hDlg);
			if (hDc)
			{
				PIXELFORMATDESCRIPTOR pfd;
				for (i = 0; i < 3; ++i)
				{
					memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
					pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
					pfd.nVersion = 1;
					pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
					pfd.iPixelType = PFD_TYPE_RGBA;
					pfd.cColorBits = 16 + (8 * i);
					pfd.cAlphaBits = 0;
					pfd.cAccumBits = 0;
					pfd.cDepthBits = 0;
					pfd.cStencilBits = 0;
					pfd.cAuxBuffers = 0;
					pfd.iLayerType = PFD_MAIN_PLANE;

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

							sprintf(itemText, "Width: %d,  Height: %d,  Bit Depth: %d", resList->width, resList->height, resList->bpp);
							SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)itemText);
							++idx;
							if (resList->width == res.width && resList->height == res.height && resList->bpp == res.bpp)
								selIdx = idx;

							break;
						}
						else if (resList->width == devMode.dmPelsWidth && resList->height == devMode.dmPelsHeight && resList->bpp == devMode.dmBitsPerPel)
						{
							break;
						}
					}
				}

				memset(&devMode, NULL, sizeof(DEVMODE));
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
					configFpsLimit = 1000.0 / val;
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

			Hooks::Patch_Video();

			EndDialog(hDlg, LOWORD(wParam));
			break;
		}

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		default:
			break;
		}

		return FALSE;
	}

	case WM_CLOSE:
		DefWindowProc(hDlg, message, wParam, lParam);
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

	if (!DialogBoxParam(hDllModule, MAKEINTRESOURCE(102), NULL, DlgProc, 0))
		exit(1);
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
	}
}