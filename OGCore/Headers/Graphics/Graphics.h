#pragma once

#include "OGCore/Properties.h"
#include "GraphicsLayer.h"
#include <Types/Color.h>
#undef RGB

namespace oi {

	namespace gc {

		class Graphics {
			
		public:

			void init(wc::Window *w) {
				glayer = GraphicsLayer<OI_GRAPHICS_TYPE>(w);
			}

			void clear(RGBf color) {
				glayer.clear(color[0], color[1], color[2]);
			}

			void clear() {
				glayer.clear(0, 0, 0);
			}

		private:

			GraphicsLayer<OI_GRAPHICS_TYPE> glayer;
		};

	}

}