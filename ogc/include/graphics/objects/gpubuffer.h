#pragma once

#include "graphicsobject.h"
#include "graphics/gl/generic.h"
#include "types/vector.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		DEnum(GPUBufferType, u32, UBO = 0, SSBO = 1, IBO = 2, VBO = 3, CBO = 4);

		class GPUBuffer;

		struct GPUBufferInfo {

			typedef GPUBuffer ResourceType;

			GPUBufferType type;
			Buffer buffer;

			bool hasData;
			std::vector<Vec2u> changes;

			//Empty gpu buffer
			GPUBufferInfo(GPUBufferType type, u32 size) : type(type), buffer(size), hasData(false) { buffer.clear(); }

			//Filled gpu buffer
			GPUBufferInfo(GPUBufferType type, Buffer buffer) : type(type), buffer(buffer), hasData(true) {}

		};

		class GPUBuffer : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			GPUBufferType getType();

			u32 getSize();
			u8 *getAddress();

			Buffer getBuffer();

			GPUBufferExt &getExtension();
			const GPUBufferInfo &getInfo();

			bool set(Buffer buf, u32 offset = 0);

			//Flush tells the GPU to update a range of the buffer
			//These changes get pushed by Graphics
			void flush(Vec2u range);

		protected:

			~GPUBuffer();
			GPUBuffer(GPUBufferInfo info);
			bool init();
			void destroy();

			//Push changes to GPU
			void push();

			bool shouldStage();

		private:

			GPUBufferInfo info;
			GPUBufferExt *ext;

		};

	}

}