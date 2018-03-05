#pragma once
#include <Types/Generic.h>
#include "Graphics/Material/ShaderInput.h"
#include "Graphics/GPU/BufferGPU.h"
#include "BufferLayoutInfo.h"

namespace oi {

	namespace gc {

		class BufferLayout : public GraphicsResource {

		public:

			BufferLayout(Graphics *&_gl, BufferLayoutInfo _info) : GraphicsResource(_gl), info(_info) {}

			BufferLayoutInfo getInfo() { return info; }

		protected:

			BufferLayoutInfo info;

		};

	}

}