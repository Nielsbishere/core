#include "graphics/commandlist.h"
#include "graphics/mesh.h"
using namespace oi::gc;
using namespace oi;

CommandList::CommandList(CommandListInfo info) : info(info) {}

bool CommandList::bind(MeshBuffer *meshBuffer) {
	return bind(meshBuffer->getInfo().vbos, meshBuffer->getInfo().ibo);
}

void CommandList::draw(Mesh *mesh, u32 instances) {

	MeshAllocation alloc = mesh->getAllocation();

	if (alloc.indices != 0)
		drawIndexed(alloc.indices, instances, alloc.baseIndex, alloc.baseVertex);
	else
		draw(alloc.vertices, instances, alloc.baseVertex);

}