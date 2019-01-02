#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/vulkan.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi;

void DrawList::prepareDrawList() {

	if (info.meshBuffer->getInfo().maxIndices == 0) {

		VkDrawIndirectCommand *drawCmd = new VkDrawIndirectCommand[getBatches()];
		VkDrawIndirectCommand *ptr = drawCmd;
		
		u32 instanceOffset = 0;

		for (auto it : info.objects) {

			ptr->firstInstance = instanceOffset;
			ptr->instanceCount = it.second;
			ptr->firstVertex = it.first->getInfo().allocation.baseVertex;
			ptr->vertexCount = it.first->getInfo().allocation.vertices;

			instanceOffset += ptr->instanceCount;
			++ptr;
		}

		info.drawBuffer->set(Buffer::construct((u8*) drawCmd, (u32) sizeof(VkDrawIndirectCommand) * getBatches()));

		delete[] drawCmd;

	} else {

		VkDrawIndexedIndirectCommand *drawCmd = new VkDrawIndexedIndirectCommand[getBatches()];
		VkDrawIndexedIndirectCommand *ptr = drawCmd;

		u32 instanceOffset = 0;

		for (auto it : info.objects) {

			ptr->firstInstance = instanceOffset;
			ptr->instanceCount = it.second;
			ptr->vertexOffset = it.first->getInfo().allocation.baseVertex;
			ptr->firstIndex = it.first->getInfo().allocation.baseIndex;
			ptr->indexCount = it.first->getInfo().allocation.indices;

			instanceOffset += ptr->instanceCount;
			++ptr;
		}

		info.drawBuffer->set(Buffer::construct((u8*)drawCmd, (u32) sizeof(VkDrawIndexedIndirectCommand) * getBatches()));

		delete[] drawCmd;

	}

}

bool DrawList::createCBO() {
	g->use(info.drawBuffer = g->create(getName() + " CBO", GPUBufferInfo(GPUBufferType::CBO, getMaxBatches() * u32(info.meshBuffer->getInfo().maxIndices != 0 ? sizeof(VkDrawIndexedIndirectCommand) : sizeof(VkDrawIndirectCommand)))));
	return true;
}

#endif