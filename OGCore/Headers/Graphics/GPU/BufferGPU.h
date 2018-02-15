#pragma once

#include <Types/Buffer.h>
#include <vector>
#include <Utils/Log.h>
#include "GraphicsResource.h"
#include "BufferInfo.h"

namespace oi {

	namespace gc {

		//Used to ensure optimal performance;
		//Write to buffer (driver buffer when initialized)
		//Sync with GPU buffer (automatic; no GL calls)
		class BufferGPU : public GraphicsResource {

		public:

			BufferGPU(BufferInfo _info) : info(_info) {}

			virtual ~BufferGPU() { getInfo().getBuffer().deconstruct(); }

			//Get part of the buffer (to read/write)
			//Length = 0; full buffer
			//subbuffer() returns full buffer
			Buffer subbuffer(u32 offset = 0, u32 length = 0) {

				if (length == 0)
					length = getInfo().size();

				if (length + offset > getInfo().size()) {
					Log::throwError<BufferGPU, 0x1>("Out of bounds");
					return Buffer();
				}

				Buffer bu = Buffer::construct(getInfo().getBuffer().addr() + offset, length);
				return bu;
			}

			BufferInfo &getInfo() { return info; }

		protected:

			BufferInfo info;

		};

	}

}