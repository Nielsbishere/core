#pragma once
#include "types/vector.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class ComputeList;
		class Pipeline;
		class GBuffer;

		struct ComputeListInfo {

			typedef ComputeList ResourceType;

			Pipeline *computePipeline;
			u32 maxDispatches;
			bool clearOnUse;

			std::vector<Vec3u> dispatches;
			GBuffer *dispatchBuffer = nullptr;

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
			GBuffer *getDispatchBuffer();

			void clear();
			void flush();

			u32 dispatchThreads(u32 threads);			//Tries to dispatch n threads, can dispatch more (group size) or less (max dispatches); returns number of dispatched threads
			Vec2u dispatchThreads(Vec2u threads);		//Tries to dispatch xy threads, can dispatch more (group size) or less (max dispatches); returns number of dispatched threads
			Vec3u dispatchThreads(Vec3u threads);		//Tries to dispatch xyz threads, can dispatch more (group size) or less (max dispatches); returns number of dispatched threads

			u32 dispatchGroups(u32 groups);				//Tries to dispatch n groups, can dispatch less (max dispatches); returns number of dispatched threads
			Vec2u dispatchGroups(Vec2u groups);			//Tries to dispatch xy groups, can dispatch less (max dispatches); returns number of dispatched threads
			Vec3u dispatchGroups(Vec3u groups);			//Tries to dispatch xyz groups, can dispatch less (max dispatches); returns number of dispatched threads

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