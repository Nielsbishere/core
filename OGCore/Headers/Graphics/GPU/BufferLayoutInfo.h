#pragma once

#include <Types/GDataType.h>
#include <Types/OString.h>

namespace oi {

	namespace gc {

		struct BufferLayoutElement {
			GDataType type;
			u32 index;
			OString buffer;
			OString name;
		};

		class BufferLayoutInfo {

		public:

			BufferLayoutInfo(std::vector<BufferLayoutElement> _elements, OString _index = "") : elements(_elements), index(_index) {}

			BufferLayoutElement operator[](u32 i) { return elements[i]; }
			OString getIndexBuffer() { return index; }

			u32 size() { return (u32) elements.size(); }

		private:

			OString index;
			std::vector<BufferLayoutElement> elements;

		};

	}

}