#include "types/buffer.h"
#include "graphics/objects/gpubuffer.h"
using namespace oi::gc;
using namespace oi;

GPUBuffer::GPUBuffer(GPUBufferInfo info) : info(info) {}
GPUBufferType GPUBuffer::getType() const { return info.type; }
u32 GPUBuffer::getSize() const { return info.buffer.size(); }
u8 *GPUBuffer::getAddress() const { return info.buffer.addr(); }
Buffer GPUBuffer::getBuffer() const { return info.buffer; }

const GPUBufferInfo &GPUBuffer::getInfo() const { return info; }

bool GPUBuffer::set(Buffer buf, u32 offset) {

	if (buf.size() + offset > info.buffer.size())
		return Log::error("GPUBuffer::set please use a buffer that matches the buffer's size");

	memcpy(info.buffer.addr() + offset, buf.addr(), buf.size());
	flush(Vec2u(0, buf.size()) + offset);
	return true;
}

GPUBuffer::~GPUBuffer() {
	info.buffer.deconstruct();
	destroy();
}