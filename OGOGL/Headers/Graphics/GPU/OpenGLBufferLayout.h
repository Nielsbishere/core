#pragma once

#include <Template/PlatformDefines.h>
#include "API/OpenGL.h"
#include <Graphics/GPU/BufferLayout.h>

namespace oi {

	namespace gc {

		class OpenGLBufferLayout : public BufferLayout {

		public:

			OpenGLBufferLayout(Graphics *&gl, BufferLayoutInfo _info);
			~OpenGLBufferLayout();

			bool init() override;
			void destroy() override;
			void bind() override;
			void unbind() override;

		private:

			GLuint gpuHandle;

		};

	}

}