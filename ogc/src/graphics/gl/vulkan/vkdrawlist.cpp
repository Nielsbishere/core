#ifdef __VULKAN__

#include "graphics/drawlist.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
#include "graphics/mesh.h"
using namespace oi::gc;
using namespace oi;

void DrawList::prepareDrawList() {

	info.drawBuffer->open();

	if (info.meshBuffer->getInfo().maxIndices == 0) {

		VkDrawIndirectCommand *drawCmd = new VkDrawIndirectCommand[getBatches()];
		VkDrawIndirectCommand *ptr = drawCmd;
		
		u32 instanceOffset = 0;

		for (auto it : info.objects) {

			ptr->firstInstance = instanceOffset;
			ptr->instanceCount = it.second.instances;
			ptr->firstVertex = it.first->getInfo().allocation.baseVertex;
			ptr->vertexCount = it.first->getInfo().allocation.vertices;

			instanceOffset += ptr->instanceCount;
			++ptr;
		}

		info.drawBuffer->copy(Buffer::construct((u8*) drawCmd, (u32) sizeof(VkDrawIndirectCommand) * getBatches()));

		delete drawCmd;

	} else {

		VkDrawIndexedIndirectCommand *drawCmd = new VkDrawIndexedIndirectCommand[getBatches()];
		VkDrawIndexedIndirectCommand *ptr = drawCmd;

		u32 instanceOffset = 0;

		for (auto it : info.objects) {

			ptr->firstInstance = instanceOffset;
			ptr->instanceCount = it.second.instances;
			ptr->vertexOffset = it.first->getInfo().allocation.baseVertex;
			ptr->firstIndex = it.first->getInfo().allocation.baseIndex;
			ptr->indexCount = it.first->getInfo().allocation.indices;

			instanceOffset += ptr->instanceCount;
			++ptr;
		}

		info.drawBuffer->copy(Buffer::construct((u8*)drawCmd, (u32) sizeof(VkDrawIndexedIndirectCommand) * getBatches()));

		delete drawCmd;

	}

	info.drawBuffer->close();

}

bool DrawList::createCBO() {
	return (info.drawBuffer = g->create(GBufferInfo(GBufferType::CBO, getMaxBatches() * u32(info.meshBuffer->getInfo().maxIndices != 0 ? sizeof(VkDrawIndexedIndirectCommand) : sizeof(VkDrawIndirectCommand))))) != nullptr;
}

#endif