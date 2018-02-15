#include "Graphics/Material/OpenGLTexture.h"
using namespace oi::gc;
using namespace oi;

OpenGLTexture::~OpenGLTexture() {
	destroy();
}

GLenum OpenGLTexture::getType() {
	return (GLenum)OpenGLTextureType(getInfo().getType().getName()).getValue();
}

GLenum OpenGLTexture::getFormat() {
	return (GLenum)OpenGLTextureFormat(getInfo().getFormat().getName()).getValue();
}

GLenum OpenGLTexture::getLayout() {
	return (GLenum)OpenGLTextureLayout(getInfo().getLayout().getName()).getValue();
}

GLenum OpenGLTexture::getSymbolic() {

	TextureLayout layout = getInfo().getLayout();

	if (layout == TextureLayout::D24_S8) return GL_UNSIGNED_INT_24_8;
	if (layout == TextureLayout::RGBA) return GL_UNSIGNED_INT_8_8_8_8_REV;
	if (layout == TextureLayout::D32f_S8) return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

	if (layout.getName().endsWith("f")) return GL_FLOAT;
	if (layout.getName().endsWith("16i") || layout.getName().endsWith("16s")) return GL_SHORT;
	if (layout.getName().endsWith("16")) return GL_UNSIGNED_SHORT;
	if (layout.getName().endsWith("32")) return GL_UNSIGNED_INT;
	if (layout.getName().endsWith("32i") || layout.getName().endsWith("32s")) return GL_INT;
	if (layout.getName().endsWith("s")) return GL_BYTE;

	return GL_UNSIGNED_BYTE;
}

bool OpenGLTexture::init() {

	glGenTextures(1, &gpuHandle);

	bind();

	GLenum type = getType();
	
	if(type == GL_TEXTURE_2D)
		glTexImage2D(type, 0, getLayout(), getInfo().getWidth(), getInfo().getHeight(), 0, getFormat(), getSymbolic(), getInfo().getBuffer().addr());

	else if (type == GL_TEXTURE_1D)
		glTexImage1D(type, 0, getLayout(), getInfo().getWidth(), 0, getFormat(), getSymbolic(), getInfo().getBuffer().addr());

	else if (type == GL_TEXTURE_3D)
		OpenGL::glTexImage3D(type, 0, getLayout(), getInfo().getWidth(), getInfo().getHeight(), getInfo().getLength(), 0, getFormat(), getSymbolic(), getInfo().getBuffer().addr());

	OpenGL::glGenerateMipmap(type);

	unbind();

	return true;
}

void OpenGLTexture::destroy() {

	if (gpuHandle != 0) {

		///TODO: Remove all resident handles
		//if (handle != 0) {
		//	//OpenGL::glMakeTextureHandleNonResidentARB(handle);
		//}

		glDeleteTextures(1, &gpuHandle);
		gpuHandle = 0;
	}

}

void OpenGLTexture::bind() {
	OpenGL::glActiveTexture(GL_TEXTURE0 + getInfo().getBinding());
	glBindTexture(getType(), gpuHandle);
}

void OpenGLTexture::unbind() {
	OpenGL::glActiveTexture(GL_TEXTURE0 + getInfo().getBinding());
	glBindTexture(getType(), 0);
}

u64 OpenGLTexture::getTextureHandle() {

	if (gpuHandle == 0) return 0;

	///TODO: Make a resident handle per sampler
	//handle = OpenGL::glGetTextureSamplerHandleARB(gpuHandle, sampler);
	//OpenGL::glMakeImageHandleNonResidentARB(handle);

	return 0;

}