#include "types/buffer.h"
#include "graphics/objects/gpubuffer.h"
using namespace oi::gc;
using namespace oi;

GPUBuffer::GPUBuffer(GPUBufferInfo info) : info(info) {}
GPUBufferType GPUBuffer::getType() { return info.type; }
u32 GPUBuffer::getSize() { return info.buffer.size(); }
u8 *GPUBuffer::getAddress() { return info.buffer.addr(); }
Buffer GPUBuffer::getBuffer() { return info.buffer; }

GPUBufferExt &GPUBuffer::getExtension() { return ext; }
const GPUBufferInfo &GPUBuffer::getInfo() { return info; }

bool GPUBuffer::set(Buffer buf, u32 offset) {

	if (buf.size() + offset > info.buffer.size())
		return Log::error("GPUBuffer::set please use a buffer that matches the buffer's size");

	memcpy(info.buffer.addr() + offset, buf.addr(), buf.size());
	flush(Vec2u(0, buf.size()) + offset);
	return true;
}