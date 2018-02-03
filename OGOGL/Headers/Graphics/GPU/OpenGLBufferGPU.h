#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferGPU.h>

namespace oi {

	namespace gc {

		class OpenGLBufferGPU : public BufferGPU { 

		public:

			OpenGLBufferGPU(BufferType type, Buffer buf);
			~OpenGLBufferGPU();

			bool init() override;
			bool update() override;

			void bind() override;
			void unbind() override;

		private:

			Buffer driverHandle;
			GLuint gpuHandle;
			GLenum arrayType;
		};

	}

}