#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
using namespace oi::gc;
using namespace oi;

const ComputeListInfo &ComputeList::getInfo() { return info; }
u32 ComputeList::getDispatches() { return (u32)info.dispatches.size(); }
u32 ComputeList::getMaxDispatches() { return info.maxDispatches; }
GBuffer *ComputeList::getDispatchBuffer() { return info.dispatchBuffer; }

void ComputeList::flush() {

	prepareComputeList();

	if (info.clearOnUse)
		clear();
}

void ComputeList::clear() {
	info.dispatches.clear();
}

Vec3u ComputeList::getThreadsPerGroup() {
	return info.computePipeline->getInfo().shader->getInfo().computeThreads;
}

Vec3u ComputeList::dispatchGroups(Vec3u groups) {

	checkDispatchGroups(groups);

	if (groups.x == 0 || groups.y == 0 || groups.z == 0)
		return Vec3u();

	info.dispatches.push_back(groups);
	return groups;
}

Vec3u ComputeList::dispatchThreads(Vec3u threads) {
	return dispatchGroups(Vec3u((Vec3d(threads) / Vec3d(getThreadsPerGroup())).ceil())) * getThreadsPerGroup();
}

Vec2u ComputeList::dispatchGroups(Vec2u groups) {
	return Vec2u(dispatchGroups(Vec3u(groups, 1)));
}

Vec2u ComputeList::dispatchThreads(Vec2u threads) {
	return dispatchGroups(Vec2u((Vec2d(threads) / Vec2d(getThreadsPerGroup())).ceil())) * Vec2u(getThreadsPerGroup());
}

u32 ComputeList::dispatchGroups(u32 groups) {
	return (dispatchGroups(Vec3u(groups, 1, 1))).x;
}

u32 ComputeList::dispatchThreads(u32 threads) {
	return dispatchGroups((u32) std::ceil((f64)threads / getThreadsPerGroup().x)) * getThreadsPerGroup().x;
}

ComputeList::ComputeList(ComputeListInfo info) : info(info) {}

ComputeList::~ComputeList() {
	g->destroy(info.dispatchBuffer);
}

bool ComputeList::init() {

	if (info.maxDispatches == 0)
		return Log::error("Couldn't create ComputeList; it needs at least 1 object");

	if (info.computePipeline == nullptr || info.computePipeline->getInfo().shader->getInfo().computeThreads == Vec3u())
		return Log::error("Couldn't create ComputeList; compute pipeline was invalid");

	info.dispatches.reserve(info.maxDispatches);

	if (!createCBO())
		return Log::error("Couldn't reserve compute list");

	return initData();
}