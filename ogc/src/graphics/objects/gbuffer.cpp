#include "types/buffer.h"
#include "graphics/objects/gbuffer.h"
using namespace oi::gc;
using namespace oi;

GBuffer::GBuffer(GBufferInfo info) : info(info) {}
GBufferType GBuffer::getType() { return info.type; }
u32 GBuffer::getSize() { return info.buffer.size(); }
u8 *GBuffer::getAddress() { return info.buffer.addr(); }
Buffer GBuffer::getBuffer() { return info.buffer; }

GBufferExt &GBuffer::getExtension() { return ext; }
const GBufferInfo &GBuffer::getInfo() { return info; }

bool GBuffer::set(Buffer buf, u32 offset) {

	if (buf.size() + offset > info.buffer.size())
		return Log::error("GBuffer::set please use a buffer that matches the gbuffer's size");

	memcpy(info.buffer.addr() + offset, buf.addr(), buf.size());
	flush(Vec2u(0, buf.size()) + offset);
	return true;
}