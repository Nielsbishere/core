#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferGPU.h>

namespace oi {

	namespace gc {

		class OpenGLBufferGPU : public BufferGPU { 

			friend class OpenGLTexture;

		public:

			OpenGLBufferGPU(BufferType type, Buffer buf);
			~OpenGLBufferGPU();

			bool init() override;

			void bind() override;
			void unbind() override;

		private:

			GLuint gpuHandle;
			GLenum arrayType;
		};

	}

}