#include "Graphics/GPU/OpenGLBufferGPU.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferGPU::OpenGLBufferGPU(BufferType type, Buffer buf, u32 binding) : BufferGPU(type, buf, binding), gpuHandle(0), arrayType(type == BufferType::IBO ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER) {}
OpenGLBufferGPU::~OpenGLBufferGPU() {
	buf.deconstruct();
	if (gpuHandle != 0) {
		OpenGL::glBindBuffer(arrayType, gpuHandle);
		OpenGL::glUnmapBuffer(arrayType);
		OpenGL::glDeleteBuffers(1, &gpuHandle);
		OpenGL::glBindBuffer(arrayType, 0);
	}
}

void OpenGLBufferGPU::destroy() {
	Buffer copyBuffer = Buffer(&buf[0], buf.size());
	buf.deconstruct();
	buf = copyBuffer;
	if (gpuHandle != 0) {
		OpenGL::glBindBuffer(arrayType, gpuHandle);
		OpenGL::glUnmapBuffer(arrayType);
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

	OpenGL::glBufferStorage(arrayType, size(), &buf[0], flags);

	u8 *dat = (u8*)OpenGL::glMapBufferRange(arrayType, 0, size(), flags);

	if (dat == nullptr) {
		OpenGL::glUnmapBuffer(arrayType);
		unbind();
		return Log::throwError<OpenGLBufferGPU, 0x1>("Couldn't generate driver handle!");
	}

	unbind();

	u32 len = size();
	buf.deconstruct();
	buf = Buffer::construct(dat, len);

	return gpuHandle != 0;
}

void OpenGLBufferGPU::bind() {
	OpenGL::glBindBuffer(arrayType, gpuHandle);
	if (type == BufferType::SSBO)
		OpenGL::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, gpuHandle);
}

void OpenGLBufferGPU::unbind() {
	OpenGL::glBindBuffer(arrayType, 0);

	if (type == BufferType::SSBO)
		OpenGL::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0);
}