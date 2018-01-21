#pragma once

#include <Types/Generic.h>
#include "OGCore/Properties.h"
#include <Types/OString.h>
#include <Types/Color.h>

namespace oi {

	namespace wc {
		class Window;
	}

	namespace gc {

		class Shader;
		class ShaderInfo;

		class Graphics {

		public:

			virtual ~Graphics() {}
			virtual bool init(wc::Window *w) = 0;
			virtual void clear() = 0;
			virtual void clear(RGBAf color) = 0;
			virtual Shader *compileShader(ShaderInfo sinf) = 0;

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