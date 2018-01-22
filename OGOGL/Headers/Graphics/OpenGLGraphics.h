#pragma once
#include <Graphics/Graphics.h>

namespace oi {

	namespace gc {

		class OpenGLGraphics : public Graphics {

		public:

			bool init(wc::Window *w) override;
			void clear() override;
			void clear(RGBAf color) override;
			Shader *compileShader(ShaderInfo sinf) override;
			BufferGPU *createBuffer(BufferType type, Buffer buf) override;

		private:

			HGLRC context, arbContext;

		};

	}

}