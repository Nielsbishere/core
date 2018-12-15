#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/gl/vulkan.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
using namespace oi::gc;
using namespace oi;

#undef min
#undef max

void ComputeList::prepareComputeList() {

	if (getDispatches() == 0)
		return;

	VkDispatchIndirectCommand *dispatchCommand = new VkDispatchIndirectCommand[getDispatches()];
	u32 dispatchSize = (u32) sizeof(VkDispatchIndirectCommand) * getDispatches();

	memcpy(dispatchCommand, info.dispatches.data(), dispatchSize);
	info.dispatchBuffer->set(Buffer::construct((u8*)dispatchCommand, dispatchSize));

	delete[] dispatchCommand;

}

bool ComputeList::createCBO() {
	g->use(info.dispatchBuffer = g->create(getName() + " CBO", GBufferInfo(GBufferType::CBO, getMaxDispatches() * u32(sizeof(VkDispatchIndirectCommand)))));
	return true;
}

struct VkComputeList {};

void ComputeList::checkDispatchGroups(Vec3u &groups) {

	VkGraphics &graphics = g->getExtension();
	Vec3u &maxComputeGroup = *(Vec3u*)(graphics.pproperties.limits.maxComputeWorkGroupCount);

	if (groups.min(maxComputeGroup) != groups)
		Log::throwError<VkComputeList, 0x0>("ComputeList::dispatch was out of bounds");

}

bool ComputeList::initData() {

	VkGraphics &graphics = g->getExtension();
	Vec3u &maxGroupSize = *(Vec3u*)(graphics.pproperties.limits.maxComputeWorkGroupSize);

	Vec3u groupSize = info.computePipeline->getInfo().shader->getInfo().computeThreads;

	if (groupSize.x * groupSize.y * groupSize.z >= graphics.pproperties.limits.maxComputeWorkGroupInvocations)
		Log::throwError<VkComputeList, 0x1>("Compute shader is invalid; the total group count is out of bounds");

	if (groupSize.min(maxGroupSize) != groupSize)
		Log::throwError<VkComputeList, 0x2>("Compute shader is invalid; group size is out of bounds");

	return true;
}

#endif