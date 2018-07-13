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
#include "GLib.h"
#include "Config.h"
#include "Main.h"

#define PREFIX_GL "gl"
#define PREFIX_WGL "wgl"

WGLGETPROCADDRESS WGLGetProcAddress;
WGLMAKECURRENT WGLMakeCurrent;
WGLCREATECONTEXT WGLCreateContext;
WGLDELETECONTEXT WGLDeleteContext;
WGLCREATECONTEXTATTRIBSARB WGLCreateContextAttribs;
WGLSWAPINTERVAL WGLSwapInterval;

GLGETSTRING GLGetString;
GLVERTEX2S GLVertex2s;
GLTEXCOORD2F GLTexCoord2f;
GLBEGIN GLBegin;
GLEND GLEnd;
GLVIEWPORT GLViewport;
GLMATRIXMODE GLMatrixMode;
GLLOADIDENTITY GLLoadIdentity;
GLORTHO GLOrtho;
GLFLUSH GLFlush;
GLFINISH GLFinish;
GLENABLE GLEnable;
GLBINDTEXTURE GLBindTexture;
GLDELETETEXTURES GLDeleteTextures;
GLTEXPARAMETERI GLTexParameteri;
GLTEXENVI GLTexEnvi;
GLTEXIMAGE1D GLTexImage1D;
GLTEXSUBIMAGE1D GLTexSubImage1D;
GLTEXIMAGE2D GLTexImage2D;
GLTEXSUBIMAGE2D GLTexSubImage2D;
GLGENTEXTURES GLGenTextures;
GLGETINTEGERV GLGetIntegerv;
GLCLEAR GLClear;
GLCLEARCOLOR GLClearColor;
GLCOLORTABLE GLColorTable;
GLPIXELSTOREI GLPixelStorei;

#ifdef _DEBUG
GLGETERROR GLGetError;
#endif

GLACTIVETEXTURE GLActiveTexture;
GLGENBUFFERS GLGenBuffers;
GLDELETEBUFFERS GLDeleteBuffers;
GLBINDBUFFER GLBindBuffer;
GLBUFFERDATA GLBufferData;
GLMAPBUFFERRANGE GLMapBufferRange;
GLUNMAPBUFFER GLUnmapBuffer;
GLFLUSHMAPPEDBUFFERRANGE GLFlushMappedBufferRange;
GLDRAWARRAYS GLDrawArrays;
GLDRAWELEMENTS GLDrawElements;
GLGENVERTEXARRAYS GLGenVertexArrays;
GLBINDVERTEXARRAY GLBindVertexArray;
GLDELETEVERTEXARRAYS GLDeleteVertexArrays;

GLENABLEVERTEXATTRIBARRAY GLEnableVertexAttribArray;
GLVERTEXATTRIBPOINTER GLVertexAttribPointer;

GLCREATESHADER GLCreateShader;
GLDELETESHADER GLDeleteShader;
GLCREATEPROGRAM GLCreateProgram;
GLSHADERSOURCE GLShaderSource;
GLCOMPILESHADER GLCompileShader;
GLATTACHSHADER GLAttachShader;
GLDETACHSHADER GLDetachShader;
GLLINKPROGRAM GLLinkProgram;
GLUSEPROGRAM GLUseProgram;
GLDELETEPROGRAM GLDeleteProgram;
GLGETSHADERIV GLGetShaderiv;
GLGETSHADERINFOLOG GLGetShaderInfoLog;

GLGETATTRIBLOCATION GLGetAttribLocation;
GLGETUNIFORMLOCATION GLGetUniformLocation;

GLUNIFORM1I GLUniform1i;
GLUNIFORMMATRIX4FV GLUniformMatrix4fv;

HMODULE hModule;

DWORD glVersion;
DWORD glPixelFormat;
DWORD glCapsClampToEdge;

BOOL isDummyRegistered;

const INT glAttributes[] = {
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
	0
};

namespace GL
{
	BOOL __fastcall Load()
	{
		if (!hModule)
			hModule = LoadLibrary("OPENGL32.dll");

		if (hModule)
		{
			WGLGetProcAddress = (WGLGETPROCADDRESS)GetProcAddress(hModule, "wglGetProcAddress");
			WGLMakeCurrent = (WGLMAKECURRENT)GetProcAddress(hModule, "wglMakeCurrent");
			WGLCreateContext = (WGLCREATECONTEXT)GetProcAddress(hModule, "wglCreateContext");
			WGLDeleteContext = (WGLDELETECONTEXT)GetProcAddress(hModule, "wglDeleteContext");
		}

		return (BOOL)hModule;
	}

	VOID __fastcall Free()
	{
		if (isDummyRegistered)
			UnregisterClass("dummyclass", hDllModule);

		if (FreeLibrary(hModule))
			hModule = NULL;
	}

	VOID __fastcall LoadGLFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func, const CHAR* sufix = NULL)
	{
		StrCopy(buffer, prefix);
		StrCat(buffer, name);

		if (sufix)
			StrCat(buffer, sufix);

		if (WGLGetProcAddress)
			*func = WGLGetProcAddress(buffer);

		if ((INT)*func >= -1 && (INT)*func <= 3)
			*func = GetProcAddress(hModule, buffer);

		if (!sufix && !*func)
		{
			LoadGLFunction(buffer, prefix, name, func, "EXT");
			if (!*func)
				LoadGLFunction(buffer, prefix, name, func, "ARB");
		}
	}

	BOOL __fastcall GetContext(HDC hDc, HGLRC* lpHRc, BOOL showError, DWORD* wglAttributes)
	{
		HGLRC hRc = WGLCreateContextAttribs(hDc, NULL, wglAttributes);
		if (hRc)
		{
			WGLMakeCurrent(hDc, hRc);
			WGLDeleteContext(*lpHRc);
			*lpHRc = hRc;

			return TRUE;
		}
		else if (showError)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_INVALID_VERSION_ARB)
				Main::ShowError("Invalid ARB version", __FILE__, __LINE__);
			else if (errorCode == ERROR_INVALID_PROFILE_ARB)
				Main::ShowError("Invalid ARB profile", __FILE__, __LINE__);
		}

		return FALSE;
	}

	BOOL __fastcall GetContext_1_4(HDC hDc, HGLRC* lpHRc, BOOL showError)
	{
		DWORD wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 1,
			WGL_CONTEXT_MINOR_VERSION_ARB, 4,
			0
		};

		return GetContext(hDc, lpHRc, showError, wglAttributes);
	}

	BOOL __fastcall GetContext_3_0(HDC hDc, HGLRC* lpHRc, BOOL showError)
	{
		DWORD wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			0
		};

		return GetContext(hDc, lpHRc, showError, wglAttributes);
	}

	VOID __fastcall CreateContextAttribs(HDC hDc, HGLRC* hRc)
	{
		CHAR buffer[256];
		LoadGLFunction(buffer, PREFIX_WGL, "CreateContextAttribs", (PROC*)&WGLCreateContextAttribs, "ARB");

		if (WGLCreateContextAttribs)
		{
			switch (configGlVersion)
			{
			case GL_VER_1:
				GetContext_1_4(hDc, hRc, TRUE);
				break;

			case GL_VER_3:
				GetContext_3_0(hDc, hRc, TRUE);
				break;

			default:
				if (!GetContext_3_0(hDc, hRc, FALSE))
					GetContext_1_4(hDc, hRc, TRUE);
				break;
			}
		}

		LoadGLFunction(buffer, "wgl", "SwapInterval", (PROC*)&WGLSwapInterval, "EXT");

		LoadGLFunction(buffer, PREFIX_GL, "GetString", (PROC*)&GLGetString);
		LoadGLFunction(buffer, PREFIX_GL, "TexCoord2f", (PROC*)&GLTexCoord2f);
		LoadGLFunction(buffer, PREFIX_GL, "Vertex2s", (PROC*)&GLVertex2s);
		LoadGLFunction(buffer, PREFIX_GL, "Begin", (PROC*)&GLBegin);
		LoadGLFunction(buffer, PREFIX_GL, "End", (PROC*)&GLEnd);
		LoadGLFunction(buffer, PREFIX_GL, "Viewport", (PROC*)&GLViewport);
		LoadGLFunction(buffer, PREFIX_GL, "MatrixMode", (PROC*)&GLMatrixMode);
		LoadGLFunction(buffer, PREFIX_GL, "LoadIdentity", (PROC*)&GLLoadIdentity);
		LoadGLFunction(buffer, PREFIX_GL, "Ortho", (PROC*)&GLOrtho);
		LoadGLFunction(buffer, PREFIX_GL, "Flush", (PROC*)&GLFlush);
		LoadGLFunction(buffer, PREFIX_GL, "Finish", (PROC*)&GLFinish);
		LoadGLFunction(buffer, PREFIX_GL, "Enable", (PROC*)&GLEnable);
		LoadGLFunction(buffer, PREFIX_GL, "BindTexture", (PROC*)&GLBindTexture);
		LoadGLFunction(buffer, PREFIX_GL, "DeleteTextures", (PROC*)&GLDeleteTextures);
		LoadGLFunction(buffer, PREFIX_GL, "TexParameteri", (PROC*)&GLTexParameteri);
		LoadGLFunction(buffer, PREFIX_GL, "TexEnvi", (PROC*)&GLTexEnvi);
		LoadGLFunction(buffer, PREFIX_GL, "TexImage1D", (PROC*)&GLTexImage1D);
		LoadGLFunction(buffer, PREFIX_GL, "TexSubImage1D", (PROC*)&GLTexSubImage1D);
		LoadGLFunction(buffer, PREFIX_GL, "TexImage2D", (PROC*)&GLTexImage2D);
		LoadGLFunction(buffer, PREFIX_GL, "TexSubImage2D", (PROC*)&GLTexSubImage2D);
		LoadGLFunction(buffer, PREFIX_GL, "GenTextures", (PROC*)&GLGenTextures);
		LoadGLFunction(buffer, PREFIX_GL, "GetIntegerv", (PROC*)&GLGetIntegerv);
		LoadGLFunction(buffer, PREFIX_GL, "Clear", (PROC*)&GLClear);
		LoadGLFunction(buffer, PREFIX_GL, "ClearColor", (PROC*)&GLClearColor);
		LoadGLFunction(buffer, PREFIX_GL, "ColorTable", (PROC*)&GLColorTable, "EXT");
		LoadGLFunction(buffer, PREFIX_GL, "PixelStorei", (PROC*)&GLPixelStorei);

#ifdef _DEBUG
		LoadGLFunction(buffer, PREFIX_GL, "GetError", (PROC*)&GLGetError);
#endif

		LoadGLFunction(buffer, PREFIX_GL, "ActiveTexture", (PROC*)&GLActiveTexture);
		LoadGLFunction(buffer, PREFIX_GL, "GenBuffers", (PROC*)&GLGenBuffers);
		LoadGLFunction(buffer, PREFIX_GL, "DeleteBuffers", (PROC*)&GLDeleteBuffers);
		LoadGLFunction(buffer, PREFIX_GL, "BindBuffer", (PROC*)&GLBindBuffer);
		LoadGLFunction(buffer, PREFIX_GL, "BufferData", (PROC*)&GLBufferData);
		LoadGLFunction(buffer, PREFIX_GL, "MapBufferRange", (PROC*)&GLMapBufferRange);
		LoadGLFunction(buffer, PREFIX_GL, "UnmapBuffer", (PROC*)&GLUnmapBuffer);
		LoadGLFunction(buffer, PREFIX_GL, "FlushMappedBufferRange", (PROC*)&GLFlushMappedBufferRange);
		LoadGLFunction(buffer, PREFIX_GL, "DrawArrays", (PROC*)&GLDrawArrays);
		LoadGLFunction(buffer, PREFIX_GL, "DrawElements", (PROC*)&GLDrawElements);
		LoadGLFunction(buffer, PREFIX_GL, "GenVertexArrays", (PROC*)&GLGenVertexArrays);
		LoadGLFunction(buffer, PREFIX_GL, "BindVertexArray", (PROC*)&GLBindVertexArray);
		LoadGLFunction(buffer, PREFIX_GL, "DeleteVertexArrays", (PROC*)&GLDeleteVertexArrays);

		LoadGLFunction(buffer, PREFIX_GL, "EnableVertexAttribArray", (PROC*)&GLEnableVertexAttribArray);
		LoadGLFunction(buffer, PREFIX_GL, "VertexAttribPointer", (PROC*)&GLVertexAttribPointer);

		LoadGLFunction(buffer, PREFIX_GL, "CreateShader", (PROC*)&GLCreateShader);
		LoadGLFunction(buffer, PREFIX_GL, "DeleteShader", (PROC*)&GLDeleteShader);
		LoadGLFunction(buffer, PREFIX_GL, "CreateProgram", (PROC*)&GLCreateProgram);
		LoadGLFunction(buffer, PREFIX_GL, "ShaderSource", (PROC*)&GLShaderSource);
		LoadGLFunction(buffer, PREFIX_GL, "CompileShader", (PROC*)&GLCompileShader);
		LoadGLFunction(buffer, PREFIX_GL, "AttachShader", (PROC*)&GLAttachShader);
		LoadGLFunction(buffer, PREFIX_GL, "DetachShader", (PROC*)&GLDetachShader);
		LoadGLFunction(buffer, PREFIX_GL, "LinkProgram", (PROC*)&GLLinkProgram);
		LoadGLFunction(buffer, PREFIX_GL, "UseProgram", (PROC*)&GLUseProgram);
		LoadGLFunction(buffer, PREFIX_GL, "DeleteProgram", (PROC*)&GLDeleteProgram);
		LoadGLFunction(buffer, PREFIX_GL, "GetShaderiv", (PROC*)&GLGetShaderiv);
		LoadGLFunction(buffer, PREFIX_GL, "GetShaderInfoLog", (PROC*)&GLGetShaderInfoLog);

		LoadGLFunction(buffer, PREFIX_GL, "GetAttribLocation", (PROC*)&GLGetAttribLocation);
		LoadGLFunction(buffer, PREFIX_GL, "GetUniformLocation", (PROC*)&GLGetUniformLocation);

		LoadGLFunction(buffer, PREFIX_GL, "Uniform1i", (PROC*)&GLUniform1i);
		LoadGLFunction(buffer, PREFIX_GL, "UniformMatrix4fv", (PROC*)&GLUniformMatrix4fv);

		const GLubyte* extensions = GLGetString(GL_EXTENSIONS);
		if (GLGetString)
		{
			glVersion = GL_VER_AUTO;

			WORD shiftVal = 8;
			const CHAR* strVer = (const CHAR*)GLGetString(GL_VERSION);
			if (strVer)
			{
				WORD j = 0;
				while (TRUE)
				{
					if (strVer[j] <= '9' && strVer[j] >= '0')
					{
						DWORD dx = (strVer[j] - '0') << shiftVal;
						glVersion += (strVer[j] - '0') << shiftVal;
						shiftVal -= 4;
					}
					else if (strVer[j] != '.')
						break;

					++j;
				}
			}
			else
				glVersion = GL_VER_1_1;

			if (configGlVersion == GL_VER_1 && glVersion >= GL_VER_1_4)
				glVersion = GL_VER_1_4;

			if (glVersion < GL_VER_1_2)
			{
				if (extensions)
					glCapsClampToEdge = (StrStr((const CHAR*)extensions, "GL_EXT_texture_edge_clamp") || StrStr((const CHAR*)extensions, "GL_SGIS_texture_edge_clamp")) ? GL_CLAMP_TO_EDGE : GL_CLAMP;
				else
					glVersion = GL_VER_1_1;
			}
			else
				glCapsClampToEdge = GL_CLAMP_TO_EDGE;
		}
		else
			glVersion = GL_VER_1_1;

		if (GLColorTable && !StrStr((const CHAR*)extensions, "GL_EXT_paletted_texture"))
			GLColorTable = NULL;
	}

	VOID __fastcall PreparePixelFormatDescription(PIXELFORMATDESCRIPTOR* pfd)
	{
		MemoryZero(pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd->nVersion = 1;
		pfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd->iPixelType = PFD_TYPE_RGBA;
		pfd->cColorBits = (BYTE)configDisplayResolution.bpp;
		pfd->iLayerType = PFD_MAIN_PLANE;
	}

	BOOL __fastcall PreparePixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		BOOL res = FALSE;

		if (!isDummyRegistered)
		{
			WNDCLASS wc = {
				CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
				DefWindowProc,
				0, 0,
				hDllModule,
				NULL, NULL,
				NULL, NULL,
				"dummyclass"
			};

			isDummyRegistered = RegisterClass(&wc);
		}

		if (isDummyRegistered)
		{
			HWND hWnd = CreateWindowEx(
				WS_EX_APPWINDOW,
				"dummyclass",
				"DUMMY",
				WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				0, 0,
				1, 1,
				NULL,
				NULL,
				hDllModule,
				NULL
			);

			if (hWnd)
			{
				HDC hDc = GetDC(hWnd);
				if (hDc)
				{
					if (!glPixelFormat)
						glPixelFormat = ChoosePixelFormat(hDc, pfd);

					if (glPixelFormat && SetPixelFormat(hDc, glPixelFormat, pfd))
					{
						HGLRC hRc = WGLCreateContext(hDc);
						if (hRc)
						{
							if (WGLMakeCurrent(hDc, hRc))
							{
								WGLCHOOSEPIXELFORMATARB WGLChoosePixelFormatARB = (WGLCHOOSEPIXELFORMATARB)WGLGetProcAddress("wglChoosePixelFormatARB");
								if (WGLChoosePixelFormatARB)
								{
									INT piFormats[128];
									UINT nNumFormats;
									if (WGLChoosePixelFormatARB(hDc, glAttributes, NULL, sizeof(piFormats) / sizeof(INT), piFormats, &nNumFormats) && nNumFormats)
										glPixelFormat = piFormats[0];
								}
								res = TRUE;

								WGLMakeCurrent(NULL, NULL);
							}

							WGLDeleteContext(hRc);
						}
					}

					ReleaseDC(hWnd, hDc);
				}

				DestroyWindow(hWnd);
			}
		}

		return res;
	}

	GLuint __fastcall CompileShaderSource(DWORD name, GLenum type)
	{
		HRSRC hResource = FindResource(hDllModule, MAKEINTRESOURCE(name), RT_RCDATA);
		if (!hResource)
			Main::ShowError("FindResource failed", __FILE__, __LINE__);

		HGLOBAL hResourceData = LoadResource(hDllModule, hResource);
		if (!hResourceData)
			Main::ShowError("LoadResource failed", __FILE__, __LINE__);

		LPVOID pData = LockResource(hResourceData);
		if (!pData)
			Main::ShowError("LockResource failed", __FILE__, __LINE__);

		GLuint shader = GLCreateShader(type);

		const GLchar* source[] = { static_cast<const GLchar*>(pData) };
		const GLint lengths[] = { SizeofResource(hDllModule, hResource) };
		GLShaderSource(shader, 1, source, lengths);

		GLShaderSource(shader, 1, source, lengths);

		GLint result;
		GLCompileShader(shader);
		GLGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			GLGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);

			if (result == 0)
				Main::ShowError("Compile shader failed", __FILE__, __LINE__);
			else
			{
				CHAR data[360];
				GLGetShaderInfoLog(shader, sizeof(data), &result, data);
				Main::ShowError(data, __FILE__, __LINE__);
			}
		}

		return shader;
	}

	DWORD __stdcall ResetThread(LPVOID lpParameter)
	{
		PIXELFORMATDESCRIPTOR pfd;
		GL::PreparePixelFormatDescription(&pfd);
		GL::PreparePixelFormat(&pfd);

		return NULL;
	}

	VOID __fastcall ResetContext()
	{
		DWORD threadId;
		SECURITY_ATTRIBUTES sAttribs;
		MemoryZero(&sAttribs, sizeof(SECURITY_ATTRIBUTES));
		sAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
		HANDLE hDummy = CreateThread(&sAttribs, NULL, ResetThread, NULL, NORMAL_PRIORITY_CLASS, &threadId);
		WaitForSingleObject(hDummy, INFINITE);
		CloseHandle(hDummy);
	}
}