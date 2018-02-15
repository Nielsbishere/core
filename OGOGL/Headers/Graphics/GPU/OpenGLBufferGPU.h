#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferGPU.h>

namespace oi {

	namespace gc {

		class OpenGLBufferGPU : public BufferGPU { 

		public:

			OpenGLBufferGPU(BufferInfo info);
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