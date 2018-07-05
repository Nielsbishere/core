#pragma once

#include <types/matrix.h>
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class GBuffer;
		class MeshBuffer;
		class Mesh;

		struct DrawListObject {
			Buffer objectBuffer;
			u32 instances;
		};

		struct DrawListInfo {

			u32 maxBatches;
			bool clearOnUse = true;

			MeshBuffer *meshBuffer;
			GBuffer *drawBuffer = nullptr;
			GBuffer *objectBuffer = nullptr;

			std::unordered_map<Mesh*, DrawListObject> objects;

			DrawListInfo(MeshBuffer *meshBuffer, GBuffer *objectBuffer, u32 maxBatches, bool clearOnUse = true) : meshBuffer(meshBuffer), objectBuffer(objectBuffer), maxBatches(maxBatches), clearOnUse(clearOnUse) {}
			DrawListInfo() : DrawListInfo(nullptr, nullptr, 0) {}

		};

		//A list of draws for a single MeshFormat
		class DrawList : public GraphicsObject {

			friend class Graphics;

		public:

			const DrawListInfo getInfo();

			u32 getBatches();
			u32 getMaxBatches();
			GBuffer *getBuffer();

			void clear();
			void flush();

			//Push instances into the draw list
			//Try avoiding calling this every time, it's better to call this function just once per mesh
			void draw(Mesh *m, u32 instances, Buffer buffer);

		protected:

			DrawList(DrawListInfo info);
			~DrawList();
			bool init();

			//Per API implementation
			//Has to put the draws into a CBO
			void prepareDrawList();

			//Per API implementation
			//Has to reserve a CBO for draw calls
			bool createCBO();

		private:

			DrawListInfo info;

		};

	}

}