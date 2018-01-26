#include "Graphics/GPU/OpenGLBufferGPU.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferGPU::OpenGLBufferGPU(BufferType type, Buffer buf) : BufferGPU(type, buf), driverHandle(Buffer()), gpuHandle(0) {}

OpenGLBufferGPU::~OpenGLBufferGPU() {
	if (initialized) {
		driverHandle.deconstruct();
		OpenGL::glBindBuffer(GL_ARRAY_BUFFER, gpuHandle);
		OpenGL::glUnmapBuffer(GL_ARRAY_BUFFER);
		OpenGL::glDeleteBuffers(1, &gpuHandle);
		OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

bool OpenGLBufferGPU::init() {

	GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	OpenGL::glGenBuffers(1, &gpuHandle);
	OpenGL::glBindBuffer(GL_ARRAY_BUFFER, gpuHandle);

	if (gpuHandle == 0)
		return Log::throwError<OpenGLBufferGPU, 0x0>("Couldn't generate gpu handle!");

	OpenGL::glBufferStorage(GL_ARRAY_BUFFER, size(), &buf[0], flags);

	u8 *dat = (u8*)OpenGL::glMapBufferRange(GL_ARRAY_BUFFER, 0, size(), flags);

	if (dat == nullptr) {
		OpenGL::glUnmapBuffer(GL_ARRAY_BUFFER);
		OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);
		return Log::throwError<OpenGLBufferGPU, 0x1>("Couldn't generate driver handle!");
	}

	OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);

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