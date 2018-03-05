#include "Graphics/OpenGLGraphics.h"
#include <Graphics/Material/ShaderInfo.h>
#include "Graphics/Material/OpenGLShader.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include "Graphics/OpenGLPrimitive.h"
#include "Graphics/GPU/OpenGLBufferLayout.h"
#include "Graphics/Material/OpenGLTexture.h"
#include "Graphics/Material/OpenGLSampler.h"
#include <Template/PlatformDefines.h>
#include <Graphics/GraphicsManager.h>
#include "API/OpenGL.h"

using namespace oi;
using namespace oi::gc;

namespace oi {
	namespace gc {
		class GraphicsManager;
	}
}

bool OpenGLGraphics::init(wc::Window *w) {

	if (!OpenGL::initWindow(w, context, arbContext) || !OpenGL::init())
		return Log::throwError<OpenGLGraphics, 0x0>("Couldn't initialize window\n");

	Log::println(OString("Initialized OpenGL context: ") + OpenGL::getVersion());
	return true;
}

void OpenGLGraphics::clear() {
	clear(Vec3(0, 0, 0));
}

Shader *OpenGLGraphics::create(OString s, ShaderInfo sinf) {

	s = OString("sha:") + s.toLowerCase();

	bool b = false;

	u32 id = manager.track(s, b);

	if (b) {
		auto *shader = new OpenGLShader(parent->gl, sinf);
		manager.set(s, shader);
		return shader;
	}

	return dynamic_cast<Shader*>(manager.get(s));
}

void OpenGLGraphics::clear(Vec3 color) {
	glClearColor(color[0], color[1], color[2], 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLGraphics::viewport(wc::Window *w) {
	glViewport(0, 0, w->getInfo().getSize().x(), w->getInfo().getSize().y());
}

BufferGPU *OpenGLGraphics::create(OString s, BufferInfo info) {

	s = OString("buf:") + s.toLowerCase();

	bool b = false;

	u32 id = manager.track(s, b);

	if (b) {
		auto *bgpu = new OpenGLBufferGPU(parent->gl, info);
		manager.set(s, bgpu);
		return bgpu;
	}

	return dynamic_cast<BufferGPU*>(manager.get(s));
}

BufferLayout *OpenGLGraphics::create(OString s, BufferLayoutInfo info) {

	s = OString("vao:") + s.toLowerCase();

	bool b = false;

	u32 id = manager.track(s, b);

	if (b) {
		auto *bly = new OpenGLBufferLayout(parent->gl, info);
		manager.set(s, bly);
		return bly;
	}

	return dynamic_cast<BufferLayout*>(manager.get(s));
}

Texture *OpenGLGraphics::create(OString s, TextureInfo info) {

	s = OString("tex:") + s.toLowerCase();

	bool b = false;

	u32 id = manager.track(s, b);

	if (b) {
		auto *tex = new OpenGLTexture(parent->gl, info);
		manager.set(s, tex);
		return tex;
	}

	return dynamic_cast<Texture*>(manager.get(s));
}

Sampler *OpenGLGraphics::create(OString s, SamplerInfo info) {

	s = OString("sam:") + s.toLowerCase();

	bool b = false;

	u32 id = manager.track(s, b);

	if (b) {
		auto *sam = new OpenGLSampler(parent->gl, info);
		manager.set(s, sam);
		return sam;
	}

	return dynamic_cast<Sampler*>(manager.get(s));
}

void OpenGLGraphics::renderElement(Primitive p, u32 length, u32 startIndex) {
	glDrawElements((GLenum)OpenGLPrimitive(p.getName()).getValue(), length, GL_UNSIGNED_INT, (GLvoid*) (u64) startIndex);
}

extern "C" __declspec(dllexport) Graphics *createGraphics(GraphicsManager *gm) {
	return new OpenGLGraphics(gm);
}