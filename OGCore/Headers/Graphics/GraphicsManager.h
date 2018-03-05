#pragma once

#include "Graphics.h"

namespace oi {

	namespace gc {

		class GraphicsManager {

		public:

			GraphicsManager();
			~GraphicsManager();

			void switchGraphics(OString path = "");

			void switchGraphics(GraphicLibrary gl);

			void endGraphics();

			void startGraphics(OString path = "");

			Graphics *gl;

			typedef Graphics *(*f_createGraphics)(GraphicsManager *gm);

		protected:

			OString current;

		private:

			//Loads the DLLs if it can find them
			//@optional OString path; loads graphics from DLL
			Graphics *load(OString path);

		};

	}

}