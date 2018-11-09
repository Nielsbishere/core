#pragma once
#include "types/generic.h"
#include "graphicsobject.h"

namespace oi {

	namespace gc {

		class Graphics;

		class GraphicsResource : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		protected:

			virtual ~GraphicsResource() {}

		};

	}
}