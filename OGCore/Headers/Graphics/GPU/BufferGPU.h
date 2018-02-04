#pragma once

#include <Types/Buffer.h>
#include <vector>
#include <Utils/Log.h>
#include "BufferType.h"

namespace oi {

	namespace gc {

		//Used to ensure optimal performance;
		//Write to first buffer
		//Update second buffer (persistent buffer / driver)
		//Updates third buffer (GPU)
		class BufferGPU {

		public:

			BufferGPU(BufferType _type, Buffer _buf) : buf(_buf), type(_type), initialized(false) {}

			virtual ~BufferGPU() { buf.deconstruct(); }

			//Upload buffer to GPU
			virtual bool init() = 0;

			virtual void bind() = 0;
			virtual void unbind() = 0;

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
			bool initialized;
			BufferType type;

		};

	}

}