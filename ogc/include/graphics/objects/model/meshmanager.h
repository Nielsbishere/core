#pragma once
#include "graphics/graphics.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class MeshBuffer;
		struct MeshBufferInfo;
		class Mesh;

		//Any number between [1, u32_MAX - 2] is used as an actual number of vertices/indices
		enum class MeshAllocationHint : u32 {

			//Force allocation into an existing MeshBuffer
			FORCE_EXISTING = u32_MAX - 1,

			//Allocates new buffer; which will have the exact size of the Mesh; !CAN'T BE CHANGED!
			SIZE_TO_FIT = u32_MAX,

			//When allocating a MeshBuffer, it will have the size defined in MeshManagerInfo
			ALLOCATE_DEFAULT = 0

		};

		struct MeshAllocationInfo {

			String name;
			String path = "";
			MeshBuffer *meshBuffer = nullptr;
			MeshAllocationHint hintMaxVertices = MeshAllocationHint::FORCE_EXISTING, hintMaxIndices = MeshAllocationHint::FORCE_EXISTING;

			std::vector<Buffer> vbos;
			Buffer ibo;

			Mesh *mesh = nullptr;

			//Path as owc-validated path, file with oiRM format
			//Allocates into MeshBuffer
			MeshAllocationInfo(String path, MeshBuffer *meshBuffer) : name(path), path(path), meshBuffer(meshBuffer) {}

			//Path as owc-validated path, file with oiRM format
			MeshAllocationInfo(String path, MeshAllocationHint hintMaxVertices = MeshAllocationHint::FORCE_EXISTING, MeshAllocationHint hintMaxIndices = MeshAllocationHint::ALLOCATE_DEFAULT) : path(path), name(path), hintMaxVertices(hintMaxVertices), hintMaxIndices(hintMaxIndices) {}

			//Allocates into VBOs and/or IBO into MeshBuffer
			MeshAllocationInfo(String name, MeshBuffer *meshBuffer, std::vector<Buffer> vbos, Buffer ibo = {}) : name(name), meshBuffer(meshBuffer), vbos(vbos), ibo(ibo) {}

			//Null buffer
			MeshAllocationInfo() : MeshAllocationInfo("", nullptr) {}


		};

		class MeshManager;

		struct MeshManagerInfo {

			typedef MeshManager ResourceType;

			u32 vertices, indices;
			TopologyMode defTopologyMode;
			FillMode defFillMode;

			std::unordered_map<String, MeshAllocationInfo> meshAllocations;

			std::vector<MeshBuffer*> meshBuffers;

			//Determines the state of an allocated MeshBuffer; how many vertices/indices can be used and what default topology/fill mode can be used.
			//By default; uses triangle fill
			MeshManagerInfo(u32 maxVertices, u32 maxIndices, TopologyMode topologyMode = TopologyMode::Triangle, FillMode fillMode = FillMode::Fill) : vertices(maxVertices), indices(maxIndices), defTopologyMode(topologyMode), defFillMode(fillMode) {}

		};

		class MeshManager : public GraphicsObject {

			friend class Graphics;

		public:

			const MeshManagerInfo &getInfo();

			//Loads one mesh (multiple should be batched with loadAll)
			Mesh *load(MeshAllocationInfo info);

			//Loads all meshes (efficiently)
			std::vector<Mesh*> loadAll(std::vector<MeshAllocationInfo> &info);

			void unload(Mesh *mesh);
			void unloadAll(MeshBuffer *meshBuffer);

			Mesh *get(String path);
			bool contains(String path);

		protected:

			~MeshManager();
			MeshManager(MeshManagerInfo info);
			bool init();

			void unload(MeshBuffer *meshBuffer, bool children);

			MeshBuffer *findBuffer(MeshBufferInfo &mbi, MeshAllocationInfo &mai);
			bool validateBuffer(MeshAllocationInfo &mai, MeshBufferInfo &mbi);

		private:

			MeshManagerInfo info;
			u32 mbId = 0;

		};

	}

}