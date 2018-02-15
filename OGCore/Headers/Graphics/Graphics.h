#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include <Types/OString.h>
#include <Types/Buffer.h>
#include <Types/Vector.h>
#include "GPU/BufferType.h"
#include "Primitive.h"

namespace oi {

	namespace wc {
		class Window;
	}

	namespace gc {

		class Shader;
		class ShaderInfo;
		class BufferGPU;
		class BufferLayout;
		class Texture;
		class TextureInfo;
		class Sampler;
		class SamplerInfo;

		class Graphics {

		public:

			virtual ~Graphics() {}
			virtual bool init(wc::Window *w) = 0;

			virtual void clear() = 0;
			virtual void clear(Vec3 color) = 0;
			virtual void viewport(wc::Window *w) = 0;

			virtual Shader *createShader(ShaderInfo sinf) = 0;
			virtual BufferGPU *createBuffer(BufferType type, Buffer b, u32 binding = 0) = 0;
			virtual Texture *createTexture(TextureInfo info, Buffer buf = Buffer::construct(nullptr, 0), u32 binding = 0) = 0;				//Textures are bindless a lot of the time, but the binding is still there
			virtual Sampler *createSampler(SamplerInfo info, u32 binding = 0) = 0;
			virtual BufferLayout *createLayout(BufferGPU *defaultBuffer) = 0;

			virtual void renderElement(Primitive p, u32 length, u32 startIndex = 0) = 0;

			typedef Graphics *(*f_createGraphics)();

			//Use this when you want to switch the graphics to a different type
			static void switchGraphics(OString path = "");

			//Use this when you want to switch the graphics to a different type
			static void switchGraphics(GraphicLibrary gl);

			//Use this when the graphic context should be removed (end of program)
			static void endGraphics();

			//Use this when the graphics context should be added
			static void startGraphics(OString path = "");
			
			static Graphics *&get();

		protected:

			static Graphics *instance;

		private:

			//Loads the DLLs if it can find them
			//@optional OString path; loads graphics from DLL
			static Graphics *load(OString path);

		};

	}
}