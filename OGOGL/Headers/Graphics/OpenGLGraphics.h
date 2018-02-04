#pragma once
#include <Graphics/Graphics.h>

namespace oi {

	namespace gc {

		class OpenGLGraphics : public Graphics {

		public:

			bool init(wc::Window *w) override;

			void viewport(wc::Window *w) override;
			void clear() override;
			void clear(RGBAf color) override;

			Shader *compileShader(ShaderInfo sinf) override;
			BufferGPU *createBuffer(BufferType type, Buffer buf) override;
			BufferLayout *createLayout(BufferGPU *defaultBuffer) override; 
			
			TextureGPU *createTexture(Vec2u res, ColorType CT, Buffer b = Buffer()) override;

			void renderElement(Primitive p, u32 length, u32 startIndex = 0) override;

		private:

			HGLRC context, arbContext;

		};

	}

}