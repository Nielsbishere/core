#include "Graphics/Material/OpenGLSampler.h"
#include "Graphics/Material/OpenGLSamplerInfo.h"
using namespace oi::gc;
using namespace oi;

OpenGLSampler::OpenGLSampler(SamplerInfo ti) : Sampler(ti) {}
OpenGLSampler::~OpenGLSampler() { destroy(); }

bool OpenGLSampler::init() {
	
	OpenGL::glGenSamplers(1, &handle);

	OpenGL::glSamplerParameteri(handle, GL_TEXTURE_WRAP_S, OpenGLSamplerWrapping(getInfo().getU().getName()).getValue());
	OpenGL::glSamplerParameteri(handle, GL_TEXTURE_WRAP_T, OpenGLSamplerWrapping(getInfo().getV().getName()).getValue());
	OpenGL::glSamplerParameteri(handle, GL_TEXTURE_WRAP_R, OpenGLSamplerWrapping(getInfo().getW().getName()).getValue());
	OpenGL::glSamplerParameteri(handle, GL_TEXTURE_MAG_FILTER, OpenGLSamplerMag(getInfo().getMagnification().getName()).getValue());
	OpenGL::glSamplerParameteri(handle, GL_TEXTURE_MIN_FILTER, OpenGLSamplerMin(getInfo().getMinification().getName()).getValue());

	if (getInfo().hasAnisotropy()) {

		GLfloat aniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

		OpenGL::glSamplerParameterf(handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	}

	return true;
}

void OpenGLSampler::destroy() {

	if (handle != 0) {
		OpenGL::glDeleteSamplers(1, &handle);
		handle = 0;
	}

}

void OpenGLSampler::bind() {
	OpenGL::glBindSampler(getInfo().getBinding(), handle);
}

void OpenGLSampler::unbind() {
	OpenGL::glBindSampler(getInfo().getBinding(), 0);
}