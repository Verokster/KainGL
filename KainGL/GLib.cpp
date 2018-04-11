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
GLCREATEPROGRAM GLCreateProgram;
GLSHADERSOURCE GLShaderSource;
GLCOMPILESHADER GLCompileShader;
GLATTACHSHADER GLAttachShader;
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

WORD glVersion;
DWORD glCapsClampToEdge;

namespace GL
{
	BOOL Load()
	{
		if (!hModule)
			hModule = LoadLibrary("OPENGL32.DLL");

		if (hModule)
		{
			WGLGetProcAddress = (WGLGETPROCADDRESS)GetProcAddress(hModule, "wglGetProcAddress");
			WGLMakeCurrent = (WGLMAKECURRENT)GetProcAddress(hModule, "wglMakeCurrent");
			WGLCreateContext = (WGLCREATECONTEXT)GetProcAddress(hModule, "wglCreateContext");
			WGLDeleteContext = (WGLDELETECONTEXT)GetProcAddress(hModule, "wglDeleteContext");
		}

		return (BOOL)hModule;
	}

	VOID Free()
	{
		if (FreeLibrary(hModule))
			hModule = NULL;
	}

	VOID __fastcall LoadGLFunction(CHAR* buffer, const CHAR* prefix, const CHAR* name, PROC* func, const CHAR* sufix = NULL)
	{
		strcpy(buffer, prefix);
		strcat(buffer, name);

		if (sufix)
			strcat(buffer, sufix);

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
			WGL_CONTEXT_FLAGS_ARB, 0,
			0
		};

		return GetContext(hDc, lpHRc, showError, wglAttributes);
	}

	BOOL __fastcall GetContext_3_0(HDC hDc, HGLRC* lpHRc, BOOL showError)
	{
		DWORD wglAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		return GetContext(hDc, lpHRc, showError, wglAttributes);
	}

	VOID CreateContextAttribs(HDC hDc, HGLRC* lpHRc)
	{
		CHAR buffer[256];
		LoadGLFunction(buffer, PREFIX_WGL, "CreateContextAttribs", (PROC*)&WGLCreateContextAttribs, "ARB");

		if (WGLCreateContextAttribs)
		{
			switch (configGlVersion)
			{
			case GL_VER_1:
				GetContext_1_4(hDc, lpHRc, TRUE);
				break;

			case GL_VER_3:
				GetContext_3_0(hDc, lpHRc, TRUE);
				break;

			default:
				if (!GetContext_3_0(hDc, lpHRc, FALSE))
					GetContext_1_4(hDc, lpHRc, TRUE);
				break;
			}
		}

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
		LoadGLFunction(buffer, PREFIX_GL, "CreateProgram", (PROC*)&GLCreateProgram);
		LoadGLFunction(buffer, PREFIX_GL, "ShaderSource", (PROC*)&GLShaderSource);
		LoadGLFunction(buffer, PREFIX_GL, "CompileShader", (PROC*)&GLCompileShader);
		LoadGLFunction(buffer, PREFIX_GL, "AttachShader", (PROC*)&GLAttachShader);
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
					glCapsClampToEdge = (strstr((const CHAR*)extensions, "GL_EXT_texture_edge_clamp") != NULL || strstr((const CHAR*)extensions, "GL_SGIS_texture_edge_clamp") != NULL) ? GL_CLAMP_TO_EDGE : GL_CLAMP;
				else
					glVersion = GL_VER_1_1;
			}
			else
				glCapsClampToEdge = GL_CLAMP_TO_EDGE;
		}
		else
			glVersion = GL_VER_1_1;

		if (GLColorTable && !strstr((const CHAR*)extensions, "GL_EXT_paletted_texture"))
			GLColorTable = NULL;
	}

	LRESULT __stdcall DummyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	BOOL PreparePixelFormat(PIXELFORMATDESCRIPTOR* pfd, DWORD* pixelFormat, HWND hWnd)
	{
		HINSTANCE hIntance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);

		CHAR* dummyclassname = "dummyclass";
		WNDCLASS wc = { NULL };
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
		wc.lpfnWndProc = DummyWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hIntance;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = dummyclassname;

		if (RegisterClass(&wc))
		{
			HWND dummyhWnd = CreateWindowEx(
				WS_EX_APPWINDOW,
				dummyclassname,
				"DUMMY",
				WS_POPUP |
				WS_CLIPSIBLINGS |
				WS_CLIPCHILDREN,
				0, 0, // Window Position
				1, 1, // Window size
				NULL, // No Parent Window
				NULL, // No Menu
				hIntance,
				NULL
			);

			if (dummyhWnd)
			{
				HDC dummyDc = ::GetDC(dummyhWnd);
				if (dummyDc)
				{
					*pixelFormat = ChoosePixelFormat(dummyDc, pfd);
					if (*pixelFormat)
					{
						if (SetPixelFormat(dummyDc, *pixelFormat, pfd))
						{
							HGLRC hRc = WGLCreateContext(dummyDc);
							if (hRc)
							{
								if (WGLMakeCurrent(dummyDc, hRc))
								{
									UINT numGlFormats;
									WGLCHOOSEPIXELFORMATARB WGLChoosePixelFormatARB = (WGLCHOOSEPIXELFORMATARB)WGLGetProcAddress("wglChoosePixelFormatARB");
									if (WGLChoosePixelFormatARB != NULL)
									{
										DWORD attribList[] = {
											WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
											WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
											WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
											WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
											WGL_COLOR_BITS_ARB, 32,
											WGL_DEPTH_BITS_ARB, 16,
											WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
											WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
											0
										};

										INT glPixelFormats[128];
										if (WGLChoosePixelFormatARB(dummyDc, (const INT*)attribList, NULL, sizeof(glPixelFormats) / sizeof(INT), glPixelFormats, &numGlFormats))
											if (numGlFormats >= 1)
												*pixelFormat = glPixelFormats[0];
									}

									if (hRc)
									{
										WGLMakeCurrent(NULL, NULL);
										WGLDeleteContext(hRc);
										hRc = NULL;
									}

									if (dummyDc != NULL)
									{
										::ReleaseDC(dummyhWnd, dummyDc);
										dummyDc = NULL;
									}

									DestroyWindow(dummyhWnd);
									UnregisterClass(dummyclassname, hIntance);

									return TRUE;
								}
							}
						}
					}
				}
			}
		}

		return FALSE;
	}

	GLuint __fastcall CompileShaderSource(const GLchar* source, GLenum type, HWND hWnd)
	{
		GLuint shader = GLCreateShader(type);

		GLint length = strlen(source);
		GLShaderSource(shader, 1, &source, &length);

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
}