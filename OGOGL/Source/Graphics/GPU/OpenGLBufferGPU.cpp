#include "Graphics/GPU/OpenGLBufferGPU.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferGPU::OpenGLBufferGPU(Graphics *&gl, BufferInfo info) : BufferGPU(gl, info), gpuHandle(0), arrayType(getInfo().getType() == BufferType::IBO ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER) {}
OpenGLBufferGPU::~OpenGLBufferGPU() {
	if (gpuHandle != 0) {
		OpenGL::glBindBuffer(arrayType, gpuHandle);
		OpenGL::glUnmapBuffer(arrayType);
		OpenGL::glDeleteBuffers(1, &gpuHandle);
		OpenGL::glBindBuffer(arrayType, 0);
	}

	getInfo().getBuffer() = Buffer::construct(nullptr, 0);
}

void OpenGLBufferGPU::destroy() {
	if (gpuHandle != 0) {
		OpenGL::glBindBuffer(arrayType, gpuHandle);

		Buffer buf = Buffer(getInfo().getBuffer().addr(), getInfo().getBuffer().size());
		OpenGL::glUnmapBuffer(arrayType);
		getInfo().getBuffer() = buf;

		OpenGL::glDeleteBuffers(1, &gpuHandle);
		OpenGL::glBindBuffer(arrayType, 0);
	}
}

bool OpenGLBufferGPU::init() {

	GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	OpenGL::glGenBuffers(1, &gpuHandle);
	bind();

	if (gpuHandle == 0)
		return Log::throwError<OpenGLBufferGPU, 0x0>("Couldn't generate gpu handle!");

	OpenGL::glBufferStorage(arrayType, getInfo().size(), getInfo().getBuffer().addr(), flags);

	u8 *dat = (u8*)OpenGL::glMapBufferRange(arrayType, 0, getInfo().size(), flags);

	if (dat == nullptr) {
		OpenGL::glUnmapBuffer(arrayType);
		unbind();
		return Log::throwError<OpenGLBufferGPU, 0x1>("Couldn't generate driver handle!");
	}

	unbind();

	u32 len = getInfo().size();
	getInfo().getBuffer().deconstruct();
	getInfo().getBuffer() = Buffer::construct(dat, len);

	return gpuHandle != 0;
}

void OpenGLBufferGPU::bind() {
	OpenGL::glBindBuffer(arrayType, gpuHandle);
	if (getInfo().getType() == BufferType::SSBO)
		OpenGL::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, getInfo().getBinding(), gpuHandle);
}

void OpenGLBufferGPU::unbind() {
	OpenGL::glBindBuffer(arrayType, 0);

	if (getInfo().getType() == BufferType::SSBO)
		OpenGL::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, getInfo().getBinding(), 0);
}