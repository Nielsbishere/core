#pragma once
#include "types/generic.h"
#include "graphics/objects/graphicsresource.h"

namespace oi {

	namespace gc {

		class Graphics;

		class TextureObject : public GraphicsResource {

		protected:

			virtual ~TextureObject() {}

		};

	}
}