#pragma once

#include "types/matrix.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class GPUBuffer;
		class MeshBuffer;
		class Mesh;

		class DrawList;

		struct DrawListInfo {

			typedef DrawList ResourceType;

			u32 maxBatches;
			bool clearOnUse = true;

			MeshBuffer *meshBuffer;
			GPUBuffer *drawBuffer = nullptr;

			std::vector<std::pair<Mesh*, u32>> objects;

			DrawListInfo(MeshBuffer *meshBuffer, u32 maxBatches, bool clearOnUse = true) : meshBuffer(meshBuffer), maxBatches(maxBatches), clearOnUse(clearOnUse) {}
			DrawListInfo() : DrawListInfo(nullptr, 0) {}

		};

		//A list of draws for a single MeshFormat
		class DrawList : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const DrawListInfo &getInfo() const;

			u32 getBatches();
			u32 getMaxBatches();
			GPUBuffer *getBuffer();

			void clear();
			void flush();

			//Push instances into the draw list
			//Try avoiding calling this every time, it's better to call this function just once per mesh
			void draw(Mesh *m, u32 instances);

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