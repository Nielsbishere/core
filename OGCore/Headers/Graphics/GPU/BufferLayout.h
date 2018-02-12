#pragma once
#include <Types/Generic.h>
#include "Graphics/Material/ShaderInput.h"
#include "Graphics/GPU/BufferGPU.h"

namespace oi {

	namespace gc {

		struct BufferLayoutElement {
			u32 size;
			ShaderInputType type;
			BufferGPU *buf;
		};

		class BufferLayout {

		public:

			BufferLayout(BufferGPU *defaultBuffer) : def(defaultBuffer) {}

			bool add(ShaderInputType type, BufferGPU *buf = nullptr) {
				
				if (buf == nullptr)
					buf = def;

				if (!ShaderInputHelper::isShaderInput(type))
					return Log::error("Couldn't add ShaderInputType; it is invalid as shader input");

				auto val = ShaderInputHelper::getType(type).getValue();

				layouts.push_back({ val.length * val.stride, type, buf });
				return true;
			}

			//When init is called with a nullptr; glDrawInstanced is not allowed
			virtual bool init(BufferGPU *indexBuffer) = 0;
			virtual void bind() = 0;
			virtual void unbind() = 0;

		protected:

			BufferGPU *def, *index;
			std::vector<BufferLayoutElement> layouts;
		};

	}

}