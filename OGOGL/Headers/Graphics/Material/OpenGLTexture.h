#pragma once

#include <Graphics/Material/Texture.h>
#include "OpenGLTextureInfo.h"

namespace oi {

	namespace gc {

		class OpenGLTexture : public Texture {


		public:

			OpenGLTexture(TextureInfo ti) : Texture(ti) {}
			~OpenGLTexture();

			bool init() override;
			void destroy() override;
			void bind() override;
			void unbind() override;

			u64 getTextureHandle() override;

			GLenum getType();
			GLenum getFormat();
			GLenum getLayout();
			GLenum getSymbolic();

		private:

			GLuint gpuHandle;

		};
	}

}