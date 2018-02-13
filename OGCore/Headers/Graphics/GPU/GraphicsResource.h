#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace gc {

		class GraphicsResource {

		public:

			virtual ~GraphicsResource() {}

			virtual bool init() = 0;		//For allocating all GPU information (not CPU!)
			virtual void destroy() = 0;		//For destroying all GPU information (not CPU!)
			virtual void bind() = 0;
			virtual void unbind() = 0;
		};

	}

}