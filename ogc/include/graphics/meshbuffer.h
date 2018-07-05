#pragma once

#include <memory/blockallocator.h>
#include "graphics/texture.h"
#include "graphics/gbuffer.h"

namespace oi {

	namespace gc {

		struct MeshAllocation {

			u32 baseVertex = 0, baseIndex = 0;
			u32 vertices = 0, indices = 0;

			std::vector<Buffer> vbo;
			Buffer ibo;

		};

		struct MeshBufferInfo {

			u32 maxVertices, maxIndices;
			std::vector<std::vector<TextureFormat>> buffers;

			VirtualBlockAllocator *vertices = nullptr, *indices = nullptr;
			std::vector<u32> vboStrides;

			std::vector<GBuffer*> vbos;
			GBuffer *ibo = nullptr;

			MeshBufferInfo(u32 maxVertices, u32 maxIndices, std::vector<std::vector<TextureFormat>> vbos) : maxVertices(maxVertices), maxIndices(maxIndices), buffers(vbos) { }
			MeshBufferInfo() : MeshBufferInfo(0, 0, {}) {}

		};

		class MeshBuffer : public GraphicsObject {

			friend class Graphics;

		public:

			const MeshBufferInfo getInfo();

			void open();								//Open for write
			void close();								//Flush updates to MeshBuffer

			//Allocate a number of vertices and/or indices.
			//If the buffer is not opened for write, it returns a null allocation.
			MeshAllocation alloc(u32 vertices, u32 indices = 0);

			//Deallocate a number of vertices and/or indices.
			bool dealloc(MeshAllocation allocation);

		protected:

			MeshBuffer(MeshBufferInfo info);
			~MeshBuffer();

			bool init();

		private:

			MeshBufferInfo info;
			bool isOpen = false;

			std::vector<Buffer> buffers;
			Buffer ibo;


		};

	}

}