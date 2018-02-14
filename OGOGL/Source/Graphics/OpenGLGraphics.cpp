#include "Graphics/OpenGLGraphics.h"
#include <Graphics/Material/ShaderInfo.h>
#include "Graphics/Material/OpenGLShader.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include "Graphics/OpenGLPrimitive.h"
#include "Graphics/GPU/OpenGLBufferLayout.h"
#include "Graphics/Material/OpenGLTexture.h"
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
	clear(Vec3(0, 0, 0));
}

Shader *OpenGLGraphics::createShader(ShaderInfo sinf) {
	return new OpenGLShader(sinf);
}

void OpenGLGraphics::clear(Vec3 color) {
	glClearColor(color[0], color[1], color[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLGraphics::viewport(wc::Window *w) {
	glViewport(0, 0, w->getInfo().getSize().x(), w->getInfo().getSize().y());
}

BufferGPU *OpenGLGraphics::createBuffer(BufferType type, Buffer buf, u32 binding) {
	return new OpenGLBufferGPU(type, buf, binding);
}

BufferLayout *OpenGLGraphics::createLayout(BufferGPU *defaultBuffer) {
	return new OpenGLBufferLayout(defaultBuffer);
}

Texture *OpenGLGraphics::createTexture(TextureInfo info, Buffer buf, u32 binding) {
	return new OpenGLTexture(info, buf, binding);
}

void OpenGLGraphics::renderElement(Primitive p, u32 length, u32 startIndex) {
	glDrawElements((GLenum)OpenGLPrimitive(p.getName()).getValue(), length, GL_UNSIGNED_INT, (GLvoid*) startIndex);
}

extern "C" __declspec(dllexport) Graphics *createGraphics() {
	return new OpenGLGraphics();
}