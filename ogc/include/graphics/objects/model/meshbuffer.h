#pragma once

#include "memory/blockallocator.h"
#include "graphics/graphics.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/shader/pipelinestate.h"

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

			std::vector<GPUBuffer*> vbos;
			GPUBuffer *ibo = nullptr;

			MeshBufferInfo(u32 maxVertices, u32 maxIndices, std::vector<std::vector<std::pair<String, TextureFormat>>> vbos, TopologyMode topologyMode = TopologyMode::Triangle, FillMode fillMode = FillMode::Fill) : maxVertices(maxVertices), maxIndices(maxIndices), buffers(vbos), topologyMode(topologyMode), fillMode(fillMode) { }
			MeshBufferInfo() : MeshBufferInfo(0, 0, {}) {}

		};

		class MeshBuffer : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const MeshBufferInfo getInfo() const;

			//Flush updates from an allocation
			void flush(const MeshAllocation &allocation);

			//Allocate a number of vertices and/or indices.
			//If the buffer is not opened for write, it returns a null allocation.
			MeshAllocation alloc(u32 vertices, u32 indices = 0);

			//Deallocate a number of vertices and/or indices.
			bool dealloc(MeshAllocation allocation);

			//Check if MeshBuffer can allocate MeshBufferInfo (a sub-buffer)
			bool canAllocate(const MeshBufferInfo &other) const;

		protected:

			MeshBuffer(MeshBufferInfo info);
			~MeshBuffer();

			bool init();

			bool sameIndices(const MeshBufferInfo &other) const;
			bool supportsModes(const MeshBufferInfo &other) const;
			bool sameFormat(const MeshBufferInfo &other) const;
			bool hasSpace(const MeshBufferInfo &other) const;

		private:

			MeshBufferInfo info;

		};

	}

}