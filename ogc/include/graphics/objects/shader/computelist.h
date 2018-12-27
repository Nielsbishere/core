#pragma once
#include "types/vector.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class ComputeList;
		class Pipeline;
		class GPUBuffer;

		struct ComputeListInfo {

			typedef ComputeList ResourceType;

			Pipeline *computePipeline;
			u32 maxDispatches;
			bool clearOnUse;

			std::vector<Vec3u> dispatches;
			GPUBuffer *dispatchBuffer = nullptr;

			ComputeListInfo(Pipeline *computePipeline, u32 maxDispatches, bool clearOnUse = false) : computePipeline(computePipeline), maxDispatches(maxDispatches), clearOnUse(clearOnUse) {}

		};

		//A list of draws for a single MeshFormat
		class ComputeList : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const ComputeListInfo &getInfo();

			u32 getMaxDispatches();
			u32 getDispatches();
			GPUBuffer *getDispatchBuffer();

			void clear();
			void flush();

			u32 dispatchThreads(u32 threads);
			Vec2u dispatchThreads(Vec2u threads);
			Vec3u dispatchThreads(Vec3u threads);

			u32 dispatchGroups(u32 groups);
			Vec2u dispatchGroups(Vec2u groups);
			Vec3u dispatchGroups(Vec3u groups);

			Vec3u getThreadsPerGroup();

		protected:

			ComputeList(ComputeListInfo info);
			~ComputeList();
			bool init();

			//Puts the dispatches into a CBO
			void prepareComputeList();

			//Reserves a CBO for draw calls
			bool createCBO();

			//Init platform dependent data
			bool initData();

			//Checks if the GPU can dispatch the groups
			void checkDispatchGroups(Vec3u &groups);

		private:

			ComputeListInfo info;

		};

	}

}