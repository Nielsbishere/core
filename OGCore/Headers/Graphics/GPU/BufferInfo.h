#pragma once

#include "BufferType.h"
#include <Types/Buffer.h>

namespace oi {

	namespace gc {

		class BufferInfo {

		public:

			BufferInfo(BufferType _type, Buffer _b, u32 _binding = 0) : type(_type), b(_b), binding(_binding) {}

			BufferType getType() { return type; }
			Buffer &getBuffer() { return b; }
			u32 getBinding() { return binding; }

			u32 size() { return b.size(); }

		private:

			BufferType type;
			Buffer b;
			u32 binding;

		};

	}

}