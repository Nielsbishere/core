#include "graphics/graphics.h"
#include "graphics/objects/model/meshbuffer.h"
using namespace oi::gc;
using namespace oi;

const MeshBufferInfo MeshBuffer::getInfo() const { return info; }

void MeshBuffer::flush(const MeshAllocation &allocation) {

	for (u32 i = 0, j = (u32) info.vboStrides.size(); i < j; ++i)
		info.vbos[i]->flush(Vec2u(allocation.baseVertex, allocation.baseVertex + allocation.vertices) * info.vboStrides[i]);

	if (info.ibo != nullptr)
		info.ibo->flush(Vec2u(allocation.baseIndex, allocation.baseIndex + allocation.indices) * 4);

}

MeshAllocation MeshBuffer::alloc(u32 vertices, u32 indices) {

	if ((indices == 0) != (info.ibo == nullptr)) {
		Log::throwError<MeshBuffer, 0x0>("Please only allocate 0 indices when there is no index buffer, or use indices when there is an index buffer");
		return {};
	}

	MeshAllocation result;
	result.vertices = vertices;
	result.indices = indices;

	if (info.ibo != nullptr){

		BlockAllocation alloc = info.indices->alloc(indices);
		result.baseIndex = alloc.start;

		if (alloc.size == 0) {
			Log::throwError<MeshBuffer, 0x1>("Couldn't allocate indices");
			return {};
		}

		result.ibo = info.ibo->getBuffer().subbuffer(alloc.start * 4, alloc.size * 4);

	}

	BlockAllocation alloc = info.vertices->alloc(vertices);
	result.baseVertex = alloc.start;

	if(alloc.size == 0) {
		Log::throwError<MeshBuffer, 0x2>("Couldn't allocate vertices");
		return {};
	}

	result.vbo.resize(info.vbos.size());

	for (u32 i = 0, j = (u32) info.vbos.size(); i < j; ++i)
		result.vbo[i] = info.vbos[i]->getBuffer().subbuffer(alloc.start * info.vboStrides[i], alloc.size * info.vboStrides[i]);

	return result;
}

bool MeshBuffer::dealloc(MeshAllocation allocation) {
	bool vdealloc = info.vertices->dealloc(allocation.baseVertex);
	bool idealloc = (info.indices != nullptr && info.indices->dealloc(allocation.baseIndex)) || info.indices == nullptr;
	return vdealloc && idealloc;
}

bool MeshBuffer::sameIndices(const MeshBufferInfo &other) const {
	return (info.maxIndices == 0) == (other.maxIndices == 0);
}

bool MeshBuffer::supportsModes(const MeshBufferInfo &other) const {
	return (other.topologyMode.getValue() == 0 || other.topologyMode == info.topologyMode) && (other.fillMode.getValue() == 0 || other.fillMode == info.fillMode);
}

bool MeshBuffer::sameFormat(const MeshBufferInfo &other) const {

	if (info.buffers.size() != other.buffers.size())
		return false;

	u32 i = 0;
	for (auto &elem : info.buffers) {

		auto &elem0 = other.buffers[i];
		if (elem.size() != elem0.size())
			return false;

		u32 j = 0;
		for (auto &elem1 : elem) {

			auto &elem2 = elem0[j];
			if (elem1.first != elem2.first || elem1.second != elem2.second)
				return false;

			++j;
		}

		++i;
	}

	return true;

}

bool MeshBuffer::hasSpace(const MeshBufferInfo &other) const {
	return info.vertices->hasSpace(other.maxVertices) && info.indices->hasSpace(other.maxIndices);
}

bool MeshBuffer::canAllocate(const MeshBufferInfo &other) const {
	return sameIndices(other) && supportsModes(other) && sameFormat(other) && hasSpace(other);
}

MeshBuffer::MeshBuffer(MeshBufferInfo info) : info(info) {}
MeshBuffer::~MeshBuffer() {

	delete info.vertices;

	for (GPUBuffer *buf : info.vbos)
		g->destroy(buf);

	if (info.indices != nullptr) {
		delete info.indices;
		g->destroy(info.ibo);
	}

}

bool MeshBuffer::init() {

	if (info.maxVertices == 0)
		return Log::error("MeshBufferInfo.maxVertices can't be zero");

	info.vertices = new VirtualBlockAllocator(info.maxVertices);
	
	if (info.maxIndices != 0) {
		info.indices = new VirtualBlockAllocator(info.maxIndices);
		info.ibo = g->create(getName() + " ibo", GPUBufferInfo(GPUBufferType::IBO, info.maxIndices * 4));
		g->use(info.ibo);
	}

	u32 i = 0;

	info.vbos.resize(info.buffers.size());
	info.vboStrides.resize(info.buffers.size());

	for (auto &buf : info.buffers) {

		u32 size = 0;

		for (auto elem : buf)
			size += Graphics::getFormatSize(elem.second);

		info.vbos[i] = g->create(getName() + " vbo " + i, GPUBufferInfo(GPUBufferType::VBO, info.maxVertices * size));
		info.vboStrides[i] = size;
		g->use(info.vbos[i]);

		++i;
	}

	return true;
}