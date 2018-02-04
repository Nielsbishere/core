#include "Graphics/OpenGLGraphics.h"
#include <Graphics/Material/ShaderInfo.h>
#include "Graphics/Material/OpenGLShader.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include "Graphics/GPU/OpenGLBufferLayout.h"
#include "Graphics/OpenGLPrimitive.h"
#include <Template/PlatformDefines.h>
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

void OpenGLGraphics::viewport(wc::Window *w) {
	glViewport(0, 0, w->getInfo().getSize().x(), w->getInfo().getSize().y());
}

BufferGPU *OpenGLGraphics::createBuffer(BufferType type, Buffer buf) {
	return new OpenGLBufferGPU(type, buf);
}

BufferLayout *OpenGLGraphics::createLayout(BufferGPU *defaultBuffer) {
	return new OpenGLBufferLayout(defaultBuffer);
}

void OpenGLGraphics::renderElement(Primitive p, u32 length, u32 startIndex) {
	glDrawElements((GLenum)OpenGLPrimitive(p.getName()).getValue(), length, GL_UNSIGNED_INT, (GLvoid*) startIndex);
}

extern "C" __declspec(dllexport) Graphics *createGraphics() {
	return new OpenGLGraphics();
}