#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace gc {

		class GraphicsResource {

		public:

			virtual ~GraphicsResource() {}

			virtual bool initData(OString path) { return true; }		//For converting the InfoObject and loading it onto CPU
			virtual bool init() = 0;									//For allocating all GPU information

			virtual void destroyData() {}								//For destroying all CPU information
			virtual void destroy() = 0;									//For destroying all GPU information

			virtual void bind() = 0;
			virtual void unbind() = 0;
		};

	}

}