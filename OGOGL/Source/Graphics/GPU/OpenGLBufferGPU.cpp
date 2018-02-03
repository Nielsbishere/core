#include "Graphics/GPU/OpenGLBufferGPU.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferGPU::OpenGLBufferGPU(BufferType type, Buffer buf) : BufferGPU(type, buf), driverHandle(Buffer()), gpuHandle(0), arrayType(type == BufferType::IBO ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER) {}

OpenGLBufferGPU::~OpenGLBufferGPU() {
	if (initialized) {
		driverHandle.deconstruct();
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
		OpenGL::glBindBuffer(arrayType, 0);
		return Log::throwError<OpenGLBufferGPU, 0x1>("Couldn't generate driver handle!");
	}

	unbind();

	driverHandle = { dat, size() };

	return initialized = true;
}

bool OpenGLBufferGPU::update() {

	for (Buffer elem : updates) {
		u32 offset = (u32)(&elem[0] - &buf[0]);
		memcpy(&driverHandle[offset], &buf[offset], elem.size());
	}

	updates.clear();

	return true;
}

void OpenGLBufferGPU::bind() {
	OpenGL::glBindBuffer(arrayType, gpuHandle);
}

void OpenGLBufferGPU::unbind() {
	OpenGL::glBindBuffer(arrayType, 0);
}