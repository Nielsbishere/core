#pragma once

#include <gl/GL.h>
#include "glext.h"
#include "wglext.h"
#include <Utils/Log.h>
#include <Window/Window.h>

namespace oi {

	namespace gc {

		class OpenGL {

		public:

			static PFNGLCREATEPROGRAMPROC glCreateProgram;
			static PFNGLDELETEPROGRAMPROC glDeleteProgram;
			static PFNGLUSEPROGRAMPROC glUseProgram;
			static PFNGLATTACHSHADERPROC glAttachShader;
			static PFNGLDETACHSHADERPROC glDetachShader;
			static PFNGLLINKPROGRAMPROC glLinkProgram;
			static PFNGLGETPROGRAMIVPROC glGetProgramiv;
			static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
			static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
			static PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
			static PFNGLUNIFORM1IPROC glUniform1i;
			static PFNGLUNIFORM1IVPROC glUniform1iv;
			static PFNGLUNIFORM2IVPROC glUniform2iv;
			static PFNGLUNIFORM3IVPROC glUniform3iv;
			static PFNGLUNIFORM4IVPROC glUniform4iv;
			static PFNGLUNIFORM1FPROC glUniform1f;
			static PFNGLUNIFORM1FVPROC glUniform1fv;
			static PFNGLUNIFORM2FVPROC glUniform2fv;
			static PFNGLUNIFORM3FVPROC glUniform3fv;
			static PFNGLUNIFORM4FVPROC glUniform4fv;
			static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
			static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
			static PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
			static PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
			static PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
			static PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
			static PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
			static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
			static PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
			static PFNGLCREATESHADERPROC glCreateShader;
			static PFNGLDELETESHADERPROC glDeleteShader;
			static PFNGLSHADERSOURCEPROC glShaderSource;
			static PFNGLCOMPILESHADERPROC glCompileShader;
			static PFNGLGETSHADERIVPROC glGetShaderiv;
			static PFNGLGENBUFFERSPROC glGenBuffers;
			static PFNGLBINDBUFFERPROC glBindBuffer;
			static PFNGLBUFFERDATAPROC glBufferData;
			static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

			static u32 getUVersion();
			static OString getVersion();
			static bool init();

			#ifdef __WINDOWS__
			static bool initWindow(wc::Window *w, HGLRC &context, HGLRC &arbContext);
			#endif

		private:

			static bool isInitialized;
			static OString version;
		};

	}

}