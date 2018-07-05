#include "graphics/meshbuffer.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

const MeshBufferInfo MeshBuffer::getInfo() { return info; }

void MeshBuffer::open() {

	u32 i = 0;

	buffers.resize(info.vbos.size());

	for (GBuffer *buf : info.vbos) {
		buf->open();
		buffers[i] = Buffer::construct(buf->getInfo().ptr, buf->getSize());
		++i;
	}

	if (info.ibo != nullptr) {
		info.ibo->open();
		ibo = Buffer::construct(info.ibo->getInfo().ptr, info.ibo->getSize());
	}

	isOpen = true;

}

void MeshBuffer::close() {

	for (u32 i = 0; i < (u32)buffers.size(); ++i) {
		info.vbos[i]->close();
		buffers[i] = {};
	}

	info.ibo->close();
	ibo = {};

	isOpen = false;

}

MeshAllocation MeshBuffer::alloc(u32 vertices, u32 indices) {

	if (!isOpen) {
		Log::throwError<MeshBuffer, 0x0>("MeshBuffer has to be opened before allocating into it");
		return {};
	}

	if ((indices == 0) != (ibo.size() == 0)) {
		Log::throwError<MeshBuffer, 0x1>("Please only allocate 0 indices when there is no index buffer, or use indices higher when there is an index buffer");
		return {};
	}

	MeshAllocation result;
	result.vertices = vertices;
	result.indices = indices;

	if (ibo.size() != 0){

		BlockAllocation alloc = info.indices->alloc(indices);
		result.baseIndex = alloc.start;

		if (alloc.size == 0) {
			Log::throwError<MeshBuffer, 0x2>("Couldn't allocate indices");
			return {};
		}

		result.ibo = ibo.subbuffer(alloc.start * 4, alloc.size * 4);

	}

	BlockAllocation alloc = info.vertices->alloc(vertices);
	result.baseVertex = alloc.start;

	if(alloc.size == 0) {
		Log::throwError<MeshBuffer, 0x3>("Couldn't allocate vertices");
		return {};
	}

	result.vbo.resize(buffers.size());

	for (u32 i = 0; i < buffers.size(); ++i)
		result.vbo[i] = buffers[i].subbuffer(alloc.start * info.vboStrides[i], alloc.size * info.vboStrides[i]);

	return result;
}

bool MeshBuffer::dealloc(MeshAllocation allocation) {
	bool vdealloc = info.vertices->dealloc(allocation.baseVertex);
	bool idealloc = (info.indices != nullptr && info.indices->dealloc(allocation.baseIndex)) || info.indices == nullptr;
	return vdealloc && idealloc;
}

MeshBuffer::MeshBuffer(MeshBufferInfo info) : info(info) {}

MeshBuffer::~MeshBuffer() {

	delete info.vertices;

	for (GBuffer *buf : info.vbos)
		g->destroy(buf);

	if (info.indices != nullptr) {
		delete info.indices;
		g->destroy(info.ibo);
	}

}

bool MeshBuffer::init() {

	if (info.maxVertices == 0)
		return Log::error("MeshBufferInfo.maxVertices can't be zero.");

	info.vertices = new VirtualBlockAllocator(info.maxVertices);
	
	if (info.maxIndices != 0) {
		info.indices = new VirtualBlockAllocator(info.maxIndices);
		info.ibo = g->create(GBufferInfo(GBufferType::IBO, info.maxIndices * 4));
	}

	for (auto &buf : info.buffers) {

		u32 size = 0;

		for (TextureFormat format : buf)
			size += Graphics::getFormatSize(format);

		info.vbos.push_back(g->create(GBufferInfo(GBufferType::VBO, info.maxVertices * size)));
		info.vboStrides.push_back(size);

	}

	return true;
}