#pragma once
#include "graphics/objects/graphicsobject.h"
#include "graphics/graphics.h"
#include "basenode.h"

namespace oi {

	namespace gc {

		class NodeSystem;
		class GPUBuffer;

		struct NodeSystemInfo {

			typedef NodeSystem ResourceType;

			u32 maxNodes, maxPhysicalNodes, maxNodesPerType;

			//Physical nodes are saved on the GPU
			//Resource nodes are stored at maxPhysicalNodes - maxNodes
			NodeSystemInfo(u32 maxNodes, u32 maxPhysicalNodes, u32 maxNodesPerType):
				maxNodes(maxNodes), maxPhysicalNodes(maxPhysicalNodes), maxNodesPerType(maxNodesPerType) {}

		};

		struct NodeSystemData {

			std::vector<BaseNode*> nodes;

			Bitset global;
			std::vector<Bitset> local;

			GPUBuffer *buffer;
			Bitset updated;	//Only for resource nodes

		};

		class NodeSystem : GraphicsObject {

		public:

			const NodeSystemInfo getInfo() const;
			const NodeSystemData &getData() const;

			bool init(BaseNode *node);
			void remove(BaseNode *node);

			void update();

		protected:

			NodeSystem(NodeSystemInfo info);
			~NodeSystem();

			bool init();

		private:

			NodeSystemInfo info;
			NodeSystemData data;

		};

	}

}