#include "Graphics/GPU/OpenGLBufferLayout.h"
#include "Graphics/Material/OpenGLShaderInput.h"
#include "Graphics/Graphics.h"
using namespace oi::gc;
using namespace oi;

OpenGLBufferLayout::OpenGLBufferLayout(Graphics *&gl, BufferLayoutInfo _info) : BufferLayout(gl, _info), gpuHandle(0) { }
OpenGLBufferLayout::~OpenGLBufferLayout() {
	destroy();
}

void OpenGLBufferLayout::destroy() {
	if (gpuHandle != 0)
		OpenGL::glDeleteVertexArrays(1, &gpuHandle);
}

bool OpenGLBufferLayout::init() {

	ResourceManager &rm = gl->getResources();

	BufferGPU *bound = nullptr;

	std::unordered_map<BufferGPU*, u32> strides(info.size());
	std::vector<u32> offsets(info.size());

	for (u32 i = 0; i < info.size(); ++i) {

		auto bufLayout = info[i];
		BufferGPU *buf = rm.get<BufferGPU>(OString("buf:") + bufLayout.buffer.toLowerCase());

		if (buf != bound)
			bound = buf;
		
		u32 &j = strides[bound];

		offsets[i] = j;
		j += bufLayout.type.getValue().length * bufLayout.type.getValue().stride;
	}

	bound = nullptr;

	OpenGL::glGenVertexArrays(1, &gpuHandle);
	bind();

	BufferGPU *index;
	if ((index = rm.get<BufferGPU>(OString("buf:") + info.getIndexBuffer().toLowerCase())) != nullptr)
		index->bind();

	for (u32 i = 0; i < info.size(); ++i) {

		auto bufLayout = info[i];
		BufferGPU *buf = rm.get<BufferGPU>(OString("buf:") + bufLayout.buffer.toLowerCase());

		if (buf != bound) {
			bound = buf;
			bound->bind();
		}

		GDataType_s val = bufLayout.type.getValue();

		OpenGL::glEnableVertexAttribArray(i);
		OpenGL::glVertexAttribPointer(i, val.length, (GLenum) OpenGLShaderInputType(ShaderInputHelper::fromType(val.derivedId).getIndex()).getValue(), GL_FALSE, (GLsizei) strides[bound], (const void*) (u64) offsets[i]);
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