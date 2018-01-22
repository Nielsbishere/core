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

			//Update buffer on GPU
			virtual bool update() = 0;

			//Replace the item at offset with the buffer thrown as second
			bool replace(u32 offset, Buffer second) {

				if (second.size() + offset > buf.size())
					return Log::throwError<BufferGPU, 0x0>("Out of bounds");

				memcpy(&buf[offset], &second[0], second.size());

				if(initialized)
					updates.push_back({ &buf[offset], second.size() });

				return true;
			}

			//Start updating a piece of the buffer
			//Don't read beyond or before [offset, offset + length] or unexpected behavior will ensue
			Buffer subbuffer(u32 offset, u32 length) {

				if (length + offset > buf.size()) {
					Log::throwError<BufferGPU, 0x1>("Out of bounds");
					return { nullptr, 0 };
				}

				Buffer bu = { &buf[offset], length };

				if (initialized)
					updates.push_back(bu);

				return bu;
			}

			//!! ONLY USE THIS FOR READING !!
			//!! NEVER USE THIS BUFFER FOR UPDATING !!
			//!! OR UNEXPECTED BEHAVIOR WILL ENSUE !!
			//Use subbuffer and replace for write
			Buffer read(u32 offset, u32 length) {
				return { &buf[offset], length };
			}

			u32 size() { return buf.size(); }
			BufferType getType() { return type; }

		protected:

			Buffer buf;
			std::vector<Buffer> updates;
			bool initialized;
			BufferType type;

		};

	}

}