#include "graphics/objects/node/nodesystem.h"
using namespace oi::gc;
using namespace oi;

const NodeSystemInfo NodeSystem::getInfo() const { return info; }
const NodeSystemData &NodeSystem::getData() const { return data; }

bool NodeSystem::init(BaseNode *node) {

	Vec2u range(0, info.maxPhysicalNodes);

	if (node->type >= NodeType::TEXTURE)
		range = Vec2u(info.maxPhysicalNodes, info.maxNodes);

	Bitset &localIds = data.local[(u32)node->type];

	for(u32 i = range.x; i < range.y; ++i)
		if (!data.global[i]) {

			for (u32 j = 0; j < info.maxNodesPerType; ++j) {
				if (!localIds[j]) {
					localIds[j] = true;
					node->localId = j;
					break;
				}
			}

			if (node->localId == u32_MAX)
				return Log::error("Couldn't allocate local id for node");

			data.global[i] = true;
			node->globalId = i;
			return true;
		}

	return Log::error("Couldn't allocate global id for node");
}

void NodeSystem::remove(BaseNode *node) {

	if (node->system != this)
		return;

	data.global[node->globalId] = false;
	data.local[node->type.getValue()][node->localId] = false;

	data.nodes[node->globalId] = nullptr;

}

void NodeSystem::update() {

}

NodeSystem::NodeSystem(NodeSystemInfo info) : info(info) {}
NodeSystem::~NodeSystem() {}

bool NodeSystem::init() {

	data.global = Bitset(info.maxNodes);
	data.updated = Bitset(info.maxPhysicalNodes);
	data.nodes.resize(info.maxNodes);
	data.local.resize(NodeType::length);

	for (u32 i = 0; i < NodeType::length; ++i)
		data.local[i] = Bitset(info.maxNodesPerType);

	//TODO: Create buffer

	return;

}