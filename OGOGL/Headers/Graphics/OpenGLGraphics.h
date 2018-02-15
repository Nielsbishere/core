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

			Shader *create(ShaderInfo sinf) override;
			BufferGPU *create(BufferInfo info) override;
			BufferLayout *create(BufferGPU *defaultBuffer) override;
			Texture *create(TextureInfo info) override;
			Sampler *create(SamplerInfo info) override;

			void renderElement(Primitive p, u32 length, u32 startIndex = 0) override;

		private:

			HGLRC context, arbContext;

		};

	}

}