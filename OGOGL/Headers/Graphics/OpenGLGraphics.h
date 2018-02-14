#pragma once
#include <Graphics/Graphics.h>

namespace oi {

	namespace gc {

		class OpenGLGraphics : public Graphics {

		public:

			bool init(wc::Window *w) override;

			void viewport(wc::Window *w) override;
			void clear() override;
			void clear(Vec3 color) override;

			Shader *createShader(ShaderInfo sinf) override;
			BufferGPU *createBuffer(BufferType type, Buffer buf, u32 binding = 0) override;
			BufferLayout *createLayout(BufferGPU *defaultBuffer) override;
			Texture *createTexture(TextureInfo info, Buffer buf = Buffer::construct(nullptr, 0), u32 binding = 0) override;

			void renderElement(Primitive p, u32 length, u32 startIndex = 0) override;

		private:

			HGLRC context, arbContext;

		};

	}

}