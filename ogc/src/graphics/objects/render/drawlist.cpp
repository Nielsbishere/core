#include "graphics/graphics.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi;

const DrawListInfo &DrawList::getInfo() const { return info; }
u32 DrawList::getBatches() { return (u32) info.objects.size(); }
u32 DrawList::getMaxBatches() { return info.maxBatches; }
GPUBuffer *DrawList::getBuffer() { return info.drawBuffer; }

void DrawList::flush() {

	prepareDrawList();

	if (info.clearOnUse)
		clear();
}

void DrawList::clear() {

	for (auto &elem : info.objects)
		g->destroy(elem.first);

	info.objects.clear();
}

void DrawList::draw(Mesh *m, u32 instances) {

	if (m->getInfo().buffer != info.meshBuffer) {
		Log::error("Every MeshBuffer requires a different DrawList. The drawcall mentioned a Mesh that wasn't in the same MeshBuffer");
		return;
	}

	auto it = std::find_if(info.objects.begin(), info.objects.end(), [m](const std::pair<Mesh*, u32> &m0) -> bool { return m0.first == m; });

	if (it == info.objects.end()) {

		if (getBatches() == getMaxBatches()) {
			Log::error("The batches exceeded the maximum amount. Please increase this or decrease draw calls");
			return;
		}

		info.objects.push_back({ m, instances });
		g->use(m);
	}
	else
		Log::throwError<DrawList, 0x0>("Grouping the meshes by instance is required!");

}

DrawList::DrawList(DrawListInfo info) : info(info) {}

DrawList::~DrawList() {
	clear();
	g->destroy(info.drawBuffer);
	g->destroy(info.meshBuffer);
}

bool DrawList::init() {

	if (info.maxBatches == 0)
		return Log::error("Couldn't create DrawList; it needs at least 1 object");

	if (info.meshBuffer == nullptr)
		return Log::error("Couldn't create DrawList; object buffer or mesh buffer was invalid");

	info.objects.reserve(info.maxBatches);

	if (!createCBO())
		return Log::error("Couldn't reserve draw list");

	g->use(info.meshBuffer);
	return true;
}