#pragma once
#include <types/generic.h>
#include "graphicsobject.h"

namespace oi {

	namespace gc {

		class Graphics;

		class GraphicsResource : public GraphicsObject {

			friend class Graphics;

		public:

			virtual ~GraphicsResource() {}

		};

	}
}