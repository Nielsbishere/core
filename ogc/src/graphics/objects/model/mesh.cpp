#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi;


const MeshInfo Mesh::getInfo() { return info; }

MeshBuffer *Mesh::getBuffer() { return info.buffer; }
MeshAllocation Mesh::getAllocation() { return info.allocation; }

Mesh::Mesh(MeshInfo info) : info(info) {}

Mesh::~Mesh() { 

	if(info.allocation.vertices != 0) 
		info.buffer->dealloc(info.allocation); 

	for (Buffer &buf : info.vbo)
		buf.deconstruct();

	info.ibo.deconstruct();

}

bool Mesh::init() {

	if ((getBuffer()->getInfo().maxIndices == 0) != (info.ibo.size() == 0))
		return Log::error("Couldn't initialize Mesh; the Mesh didn't have the same IBO settings as the MeshBuffer");

	if (info.vbo.size() != getBuffer()->getInfo().vbos.size())
		return Log::error("Couldn't initialize Mesh; there weren't enough vertex buffers");


	u32 vertices = 0;

	for (u32 i = 0; i < (u32)info.vbo.size(); ++i) {

		u32 stride = getBuffer()->getInfo().vboStrides[i];

		if(info.vbo[i].size() == 0 || info.vbo[i].size() % stride != 0)
			return Log::error("Couldn't initialize Mesh; one of the vertex buffers was incorrect");

		u32 nVertices = info.vbo[i].size() / stride;

		if(i != 0 && vertices != nVertices)
			return Log::error("Couldn't initialize Mesh; one of the vertex buffers vertices didn't match another");

		vertices = nVertices;

	}

	info.allocation = info.buffer->alloc(vertices, info.ibo.size() / 4);

	if(info.allocation.vertices == 0)
		return Log::error("Couldn't initialize Mesh; no space left in the MeshBuffer");

	if (info.ibo.size() != 0)
		info.allocation.ibo.copy(info.ibo);

	for (u32 i = 0; i < info.vbo.size(); ++i)
		info.allocation.vbo[i].copy(info.vbo[i]);

	return true;
}