#pragma once

#include "meshbuffer.h"

namespace oi {

	namespace gc {

		class Mesh;

		struct MeshInfo {

			typedef Mesh ResourceType;

			u32 vertices, indices;

			MeshBuffer *buffer;
			std::vector<Buffer> vbo;
			Buffer ibo;

			MeshAllocation allocation;

			MeshInfo(MeshBuffer *buffer, u32 vertices, u32 indices, std::vector<Buffer> vbo, Buffer ibo = {}) : buffer(buffer), vbo(vbo), ibo(ibo), vertices(vertices), indices(indices) { }

			MeshInfo() : MeshInfo(nullptr, 0, 0, {}) {}

		};

		class Mesh : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const MeshInfo &getInfo() const;

			MeshBuffer *getBuffer() const;
			MeshAllocation getAllocation() const;
			u32 getVertices() const;
			u32 getIndices() const;
			u32 getAllocationId() const;

		protected:

			Mesh(MeshInfo info);
			~Mesh();

			bool init();

		private:

			MeshInfo info;

		};

	}

}