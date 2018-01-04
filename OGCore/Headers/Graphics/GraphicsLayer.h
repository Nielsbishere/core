#pragma once

#include <Types/Generic.h>

namespace oi {

	namespace gc {

		//GraphicsLayer is for common functions that return platform dependent values
		//Graphics is a wrapper that ensures that those values are used correctly
		//gid = 0 is OpenGL4.5
		//gid = 1 is DX11
		template<u32 gid>
		struct GraphicsLayer { };


		#ifdef CAN_HANDLE_OPENGL
		//OpenGL4.5
		template<>
		struct GraphicsLayer<0> {

			GraphicsLayer() {}

			GraphicsLayer(wc::Window *w) {
				if (!OpenGL::initWindow(w, context, arb) || !OpenGL::init())
					Log::throwError<GraphicsLayer<0>, 0x0>("Couldn't init OpenGL");
				else
					Log::println(OString("Initialized OpenGL with version ") + OpenGL::getVersion());
			}

			void clear(f32 r, f32 g, f32 b) {
				glClearColor(r, g, b, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}

		private:

			HGLRC context, arb;

		};
		#endif

		#ifdef CAN_HANDLE_DX11
		//DX11
		template<>
		struct GraphicsLayer<1> {

		};
		#endif

	}
}