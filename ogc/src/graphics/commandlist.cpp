#include "graphics/commandlist.h"
#include "graphics/mesh.h"
using namespace oi::gc;
using namespace oi;

CommandList::CommandList(CommandListInfo info) : info(info) {}

bool CommandList::bind(MeshBuffer *meshBuffer) {
	return bind(meshBuffer->getInfo().vbos, meshBuffer->getInfo().ibo);
}