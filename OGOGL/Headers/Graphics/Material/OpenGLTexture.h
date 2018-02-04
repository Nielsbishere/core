#pragma once

#include <Graphics/Material/TextureGPU.h>
#include "API/OpenGL.h"

namespace oi {

	namespace gc {

		class OpenGLTexture : public TextureGPU {

		public:

			OpenGLTexture(Vec2u dim, ColorType format, BufferGPU *resource) : TextureGPU(dim, format, resource), gpuName(0), gpuHandle(0) {}
			~OpenGLTexture();

			bool init() override;
			u64 getHandle() override;
			void bind() override;
			void unbind() override;

		private:

			GLuint gpuName;
			u64 gpuHandle;
		};

	}
}