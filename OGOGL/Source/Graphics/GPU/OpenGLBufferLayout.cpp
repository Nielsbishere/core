#include "Graphics/GPU/OpenGLBufferLayout.h"
#include "Graphics/Material/OpenGLShaderInput.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferLayout::OpenGLBufferLayout(BufferGPU *defBuf): BufferLayout(defBuf), gpuHandle(0) { }
OpenGLBufferLayout::~OpenGLBufferLayout() {
	if (gpuHandle != 0)
		OpenGL::glDeleteVertexArrays(1, &gpuHandle);
}

bool OpenGLBufferLayout::init(BufferGPU *indBuf) {

	BufferGPU *bound = def;

	if (bound == nullptr) return Log::error("Default bound buffer can't be nullptr");

	std::unordered_map<BufferGPU*, u32> strides(layouts.size());
	std::vector<u32> offsets(layouts.size());

	u32 i = 0;
	for (auto bufLayout : layouts) {

		if (bufLayout.buf != bound)
			bound = bufLayout.buf;
		
		u32 &j = strides[bound];

		offsets[i] = j;
		j += bufLayout.size;
		++i;
	}

	i = 0;
	bound = def;

	OpenGL::glGenVertexArrays(1, &gpuHandle);
	bind();

	if ((index = indBuf) != nullptr)
		index->bind();

	bound->bind();

	for (auto bufLayout : layouts) {

		if (bufLayout.buf != bound) {
			bound = bufLayout.buf;
			bound->bind();
		}

		auto val = ShaderInputHelper::getType(bufLayout.type).getValue();

		OpenGL::glEnableVertexAttribArray(i);
		OpenGL::glVertexAttribPointer(i, val.length, (GLenum) OpenGLShaderInputType(ShaderInputHelper::fromType(val.derivedId).getIndex()).getValue(), GL_FALSE, (GLsizei) strides[bound], (const void*) offsets[i]);

		++i;
	}

	unbind();
	bound->unbind();
	if (index != nullptr)
		index->unbind();

	return true;
}

void OpenGLBufferLayout::bind() {
	OpenGL::glBindVertexArray(gpuHandle);
}

void OpenGLBufferLayout::unbind() {
	OpenGL::glBindVertexArray(0);
}