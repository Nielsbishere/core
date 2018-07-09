#pragma once

#include "meshbuffer.h"

namespace oi {

	namespace gc {

		struct MeshInfo {

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

		public:

			const MeshInfo getInfo();

			MeshBuffer *getBuffer();
			MeshAllocation getAllocation();

		protected:

			Mesh(MeshInfo info);
			~Mesh();

			bool init();

		private:

			MeshInfo info;

		};

	}

}