#include "Graphics/OpenGLGraphics.h"
#include <Graphics/Material/ShaderInfo.h>
#include "Graphics/Material/OpenGLShader.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include "API/OpenGL.h"

using namespace oi;
using namespace oi::gc;

bool OpenGLGraphics::init(wc::Window *w) {

	if (!OpenGL::initWindow(w, context, arbContext) || !OpenGL::init())
		return Log::throwError<OpenGLGraphics, 0x0>("Couldn't initialize window\n");

	Log::println(OString("Initialized OpenGL context: ") + OpenGL::getVersion());
	return true;
}

void OpenGLGraphics::clear() {
	clear(RGBAf(0, 0, 0, 0));
}

Shader *OpenGLGraphics::compileShader(ShaderInfo sinf) {
	OpenGLShader *shader = new OpenGLShader();
	if (!shader->init(sinf)) {
		delete shader;
		return nullptr;
	}
	return shader;
}

void OpenGLGraphics::clear(RGBAf color) {
	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

BufferGPU *OpenGLGraphics::createBuffer(BufferType type, Buffer buf) {
	return new OpenGLBufferGPU(type, buf);
}

extern "C" __declspec(dllexport) Graphics *createGraphics() {
	return new OpenGLGraphics();
}