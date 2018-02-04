#include "OGCore/Properties.h"
#include "API/OpenGL.h"

#pragma comment(lib, "OSTLCore.lib")
#pragma comment(lib, "OWCore.lib")
#pragma comment(lib, "opengl32.lib")

using namespace oi::gc;
using namespace oi;

bool OpenGL::isInitialized = false;
OString OpenGL::version = "";

PFNGLCREATEPROGRAMPROC OpenGL::glCreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC OpenGL::glDeleteProgram = nullptr;
PFNGLUSEPROGRAMPROC OpenGL::glUseProgram = nullptr;
PFNGLATTACHSHADERPROC OpenGL::glAttachShader = nullptr;
PFNGLDETACHSHADERPROC OpenGL::glDetachShader = nullptr;
PFNGLLINKPROGRAMPROC OpenGL::glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC OpenGL::glGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC OpenGL::glGetShaderInfoLog = nullptr;
PFNGLGETUNIFORMLOCATIONPROC OpenGL::glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC OpenGL::glUniform1i = nullptr;
PFNGLUNIFORM1IVPROC OpenGL::glUniform1iv = nullptr;
PFNGLUNIFORM2IVPROC OpenGL::glUniform2iv = nullptr;
PFNGLUNIFORM3IVPROC OpenGL::glUniform3iv = nullptr;
PFNGLUNIFORM4IVPROC OpenGL::glUniform4iv = nullptr;
PFNGLUNIFORM1FPROC OpenGL::glUniform1f = nullptr;
PFNGLUNIFORM1FVPROC OpenGL::glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC OpenGL::glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC OpenGL::glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC OpenGL::glUniform4fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC OpenGL::glUniformMatrix4fv = nullptr;
PFNGLGETATTRIBLOCATIONPROC OpenGL::glGetAttribLocation = nullptr;
PFNGLVERTEXATTRIB1FPROC OpenGL::glVertexAttrib1f = nullptr;
PFNGLVERTEXATTRIB1FVPROC OpenGL::glVertexAttrib1fv = nullptr;
PFNGLVERTEXATTRIB2FVPROC OpenGL::glVertexAttrib2fv = nullptr;
PFNGLVERTEXATTRIB3FVPROC OpenGL::glVertexAttrib3fv = nullptr;
PFNGLVERTEXATTRIB4FVPROC OpenGL::glVertexAttrib4fv = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC OpenGL::glEnableVertexAttribArray = nullptr;
PFNGLBINDATTRIBLOCATIONPROC OpenGL::glBindAttribLocation = nullptr;
PFNGLCREATESHADERPROC OpenGL::glCreateShader = nullptr;
PFNGLDELETESHADERPROC OpenGL::glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC OpenGL::glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC OpenGL::glCompileShader = nullptr;
PFNGLGETSHADERIVPROC OpenGL::glGetShaderiv = nullptr;
PFNGLGENBUFFERSPROC OpenGL::glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC OpenGL::glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC OpenGL::glBufferData = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC OpenGL::glVertexAttribPointer = nullptr;
PFNGLGETACTIVEUNIFORMPROC OpenGL::glGetActiveUniform = nullptr;
PFNGLGETACTIVEATTRIBPROC OpenGL::glGetActiveAttrib = nullptr;
PFNGLBINDVERTEXARRAYPROC OpenGL::glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC OpenGL::glDeleteVertexArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC OpenGL::glGenVertexArrays = nullptr;
PFNGLDELETEBUFFERSPROC OpenGL::glDeleteBuffers = nullptr;
PFNGLBUFFERSTORAGEPROC OpenGL::glBufferStorage = nullptr;
PFNGLUNMAPBUFFERPROC OpenGL::glUnmapBuffer = nullptr;
PFNGLMAPBUFFERRANGEPROC OpenGL::glMapBufferRange = nullptr;
PFNGLDELETESYNCPROC OpenGL::glDeleteSync = nullptr;
PFNGLFENCESYNCPROC OpenGL::glFenceSync = nullptr;
PFNGLCLIENTWAITSYNCPROC OpenGL::glClientWaitSync = nullptr;
PFNGLTEXBUFFERPROC OpenGL::glTexBuffer = nullptr;
PFNGLGETTEXTUREHANDLEARBPROC OpenGL::glGetTextureHandleARB = nullptr;
//PFNxPROC OpenGL::x = nullptr;

OString OpenGL::getVersion() {
	return version;
}

u32 OpenGL::getUVersion() {

	GLint minor, major;

	if(version == "")
		version = (const char*)glGetString(GL_VERSION);

	major = version[0] - '0';
	if (major >= 3) {
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
	}
	else
		minor = version[2] - '0';

	return ((u32)major << 16) | ((u32)minor & 0xFFFF);
}

#ifdef __WINDOWS__
bool OpenGL::initWindow(wc::Window *w, HGLRC &hrc, HGLRC &arb) {

	wc::WindowHandle &wh = w->getHandle();
	HWND handle = wh.window;
	HDC hdc = GetDC(handle);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(hdc, &pfd);

	if (nPixelFormat == 0) 
		return Log::error("Couldn't create pixel format");

	BOOL bResult = SetPixelFormat(hdc, nPixelFormat, &pfd);

	if (!bResult) return Log::error("Couldn't set pixel format");

	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	u32 version = getUVersion();
	u32 minor = version & 0xFFFF;
	u32 major = (version & 0xFFFF0000) >> 16;

	if (major < 4 || (major == 4 && minor < 5))
		return Log::error(OString("OpenGL 4.5 not supported (") + OString((i32)major) + "." + OString((i32)minor) + " supported)");

	int attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, (GLint)major,
		WGL_CONTEXT_MINOR_VERSION_ARB, (GLint)minor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	if (wglCreateContextAttribsARB != NULL)
		arb = wglCreateContextAttribsARB(hdc, 0, attribs);

	if (!arb || !hrc) return Log::error("OpenGL 4.5+ HRC not supported");
	
	wglMakeCurrent(hdc, arb);

	return true;
}
#endif

bool OpenGL::init() {

	if (isInitialized) return true;

	// Program
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)wglGetProcAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)wglGetProcAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)wglGetProcAddress("glUniform4iv");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)wglGetProcAddress("glVertexAttrib1f");
	glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)wglGetProcAddress("glVertexAttrib1fv");
	glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)wglGetProcAddress("glVertexAttrib2fv");
	glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)wglGetProcAddress("glVertexAttrib3fv");
	glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)wglGetProcAddress("glVertexAttrib4fv");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");

	// Shader
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)wglGetProcAddress("glGetActiveUniform");
	glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)wglGetProcAddress("glGetActiveAttrib");


	//Buffer
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)wglGetProcAddress("glMapBufferRange");
	glBufferStorage = (PFNGLBUFFERSTORAGEPROC)wglGetProcAddress("glBufferStorage");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glDeleteSync = (PFNGLDELETESYNCPROC)wglGetProcAddress("glDeleteSync");
	glFenceSync = (PFNGLFENCESYNCPROC)wglGetProcAddress("glFenceSync");
	glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)wglGetProcAddress("glClientWaitSync");
	glTexBuffer = (PFNGLTEXBUFFERPROC)wglGetProcAddress("glTexBuffer");
	glGetTextureHandleARB = (PFNGLGETTEXTUREHANDLEARBPROC)wglGetProcAddress("glGetTextureHandleARB");

	return isInitialized = true;
}