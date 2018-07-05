#pragma once

#include "meshbuffer.h"

namespace oi {

	namespace gc {

		struct MeshInfo {

			//String origin;

			MeshBuffer *buffer;
			std::vector<Buffer> vbo;
			Buffer ibo;

			MeshAllocation allocation;

			MeshInfo(MeshBuffer *buffer, std::vector<Buffer> vbo, Buffer ibo = {}) : buffer(buffer), vbo(vbo), ibo(ibo) {}
			MeshInfo() : MeshInfo(nullptr, {}) {}

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