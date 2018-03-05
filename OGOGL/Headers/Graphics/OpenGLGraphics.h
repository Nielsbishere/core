#pragma once
#include <Graphics/Graphics.h>

namespace oi {

	namespace gc {

		class OpenGLGraphics : public Graphics {

		public:

			OpenGLGraphics(GraphicsManager *gm) : Graphics(gm) {}
			~OpenGLGraphics() {}

			bool init(wc::Window *w) override;

			void viewport(wc::Window *w) override;
			void clear() override;
			void clear(Vec3 color) override;

			Shader *create(OString s, ShaderInfo sinf) override;
			BufferGPU *create(OString s, BufferInfo info) override;
			BufferLayout *create(OString s, BufferLayoutInfo info) override;
			Texture *create(OString s, TextureInfo info) override;
			Sampler *create(OString s, SamplerInfo info) override;

			void renderElement(Primitive p, u32 length, u32 startIndex = 0) override;

		private:

			HGLRC context, arbContext;

		};

	}

}