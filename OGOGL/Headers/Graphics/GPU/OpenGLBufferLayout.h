#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferLayout.h>

namespace oi {

	namespace gc {

		class OpenGLBufferLayout : public BufferLayout {

		public:

			OpenGLBufferLayout(BufferGPU *defBuf);
			~OpenGLBufferLayout();

			bool init(BufferGPU *indBuf) override;
			void bind() override;
			void unbind() override;

		private:

			GLuint gpuHandle;

		};

	}

}