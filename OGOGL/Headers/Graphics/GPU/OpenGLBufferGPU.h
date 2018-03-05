#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferGPU.h>

namespace oi {

	namespace gc {

		class OpenGLBufferGPU : public BufferGPU {

			friend class OpenGLShader;

		public:

			OpenGLBufferGPU(Graphics *&gl, BufferInfo info);
			~OpenGLBufferGPU();

			bool init() override;
			void destroy() override;

			void bind() override;
			void unbind() override;

		private:

			GLuint gpuHandle;
			GLenum arrayType;
		};

	}

}