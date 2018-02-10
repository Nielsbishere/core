#pragma once

#include <Types/Buffer.h>
#include <vector>
#include <Utils/Log.h>
#include "BufferType.h"
#include "GraphicsResource.h"

namespace oi {

	namespace gc {

		//Used to ensure optimal performance;
		//Write to buffer (driver buffer when initialized)
		//Sync with GPU buffer (automatic; no GL calls)
		class BufferGPU : public GraphicsResource {

		public:

			BufferGPU(BufferType _type, Buffer _buf, u32 _binding = 0) : buf(_buf), type(_type), binding(_binding) {}

			virtual ~BufferGPU() { buf.deconstruct(); }

			//Get part of the buffer (to read/write)
			//Length = 0; full buffer
			//subbuffer() returns full buffer
			Buffer subbuffer(u32 offset = 0, u32 length = 0) {

				if (length == 0)
					length = size();

				if (length + offset > buf.size()) {
					Log::throwError<BufferGPU, 0x1>("Out of bounds");
					return Buffer();
				}

				Buffer bu = Buffer::construct(&buf[offset], length);
				return bu;
			}

			u32 size() { return buf.size(); }
			BufferType getType() { return type; }

		protected:

			Buffer buf;
			BufferType type;
			u32 binding;

		};

	}

}