#pragma once

#include "memory/blockallocator.h"
#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/texture/texture.h"

namespace oi {

	namespace gc {

		struct MeshAllocation {

			u32 baseVertex = 0, baseIndex = 0;
			u32 vertices = 0, indices = 0;

			std::vector<Buffer> vbo;
			Buffer ibo;

		};

		class MeshBuffer;

		struct MeshBufferInfo {

			typedef MeshBuffer ResourceType;

			u32 maxVertices, maxIndices;
			std::vector<std::vector<std::pair<String, TextureFormat>>> buffers;
			TopologyMode topologyMode;
			FillMode fillMode;

			VirtualBlockAllocator *vertices = nullptr, *indices = nullptr;
			std::vector<u32> vboStrides;

			std::vector<GBuffer*> vbos;
			GBuffer *ibo = nullptr;

			MeshBufferInfo(u32 maxVertices, u32 maxIndices, std::vector<std::vector<std::pair<String, TextureFormat>>> vbos, TopologyMode topologyMode = TopologyMode::Triangle, FillMode fillMode = FillMode::Fill) : maxVertices(maxVertices), maxIndices(maxIndices), buffers(vbos), topologyMode(topologyMode), fillMode(fillMode) { }
			MeshBufferInfo() : MeshBufferInfo(0, 0, {}) {}

		};

		class MeshBuffer : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const MeshBufferInfo getInfo();

			void flush();		//Flush updates to MeshBuffer

			//Allocate a number of vertices and/or indices.
			//If the buffer is not opened for write, it returns a null allocation.
			MeshAllocation alloc(u32 vertices, u32 indices = 0);

			//Deallocate a number of vertices and/or indices.
			bool dealloc(MeshAllocation allocation);

			//Check if MeshBuffer can allocate MeshBufferInfo (a sub-buffer)
			bool canAllocate(const MeshBufferInfo &other);

		protected:

			MeshBuffer(MeshBufferInfo info);
			~MeshBuffer();

			bool init();

			bool sameIndices(const MeshBufferInfo &other);
			bool supportsModes(const MeshBufferInfo &other);
			bool sameFormat(const MeshBufferInfo &other);
			bool hasSpace(const MeshBufferInfo &other);

		private:

			MeshBufferInfo info;

		};

	}

}