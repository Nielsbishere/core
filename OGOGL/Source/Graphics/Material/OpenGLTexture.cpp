#include "Graphics/Material/OpenGLTexture.h"
#include "Graphics/GPU/OpenGLBufferGPU.h"
#include <Template/PlatformDefines.h>
#include "API/OpenGL.h"
using namespace oi::gc;
using namespace oi;

OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &gpuName);
}

GLenum getInternalFormat(ColorType CT) {

	switch (CT) {

	case ColorType::COMPRESSED_R:
		return GL_R8;

	case ColorType::COMPRESSED_RG:
		return GL_RG8;

	case ColorType::COMPRESSED_RGB:
		return GL_RGB8;

	case ColorType::NORMAL_R:
		return GL_R32F;

	case ColorType::NORMAL_RG:
		return GL_RG32F;

	case ColorType::NORMAL_RGB:
		return GL_RGB32F;

	case ColorType::NORMAL_RGBA:
		return GL_RGB32F;

	case ColorType::EXTENDED_R:
		return Log::error("Extended textures are CPU only");

	case ColorType::EXTENDED_RG:
		return Log::error("Extended textures are CPU only");

	case ColorType::EXTENDED_RGB:
		return Log::error("Extended textures are CPU only");

	case ColorType::EXTENDED_RGBA:
		return Log::error("Extended textures are CPU only");

	default:
		return GL_UNSIGNED_INT_8_8_8_8_REV;
	}
}

bool OpenGLTexture::init() {

	if (!gpuResource->init())
		return Log::error("Couldn't init texture; buffer init failed");

	glGenTextures(1, &gpuName);

	bind();

	GLenum intern = getInternalFormat(getFormat());

	if (intern == 0)
		return Log::error("Couldn't find texture format");

	OpenGL::glTexBuffer(GL_TEXTURE_BUFFER, intern, ((OpenGLBufferGPU*) gpuResource)->gpuHandle);

	gpuHandle = OpenGL::glGetTextureHandleARB(gpuName);

	unbind();

	return true;
}

u64 OpenGLTexture::getHandle() {
	return gpuHandle;
}

void OpenGLTexture::bind() {
	glBindTexture(GL_TEXTURE_BUFFER, gpuName);
}

void OpenGLTexture::unbind() {
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}