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

bool GBuffer::copy(Buffer buf) {

	if (buf.size() > info.buffer.size())
		return Log::error("GBuffer::set please use a buffer that matches the gbuffer's size");

	memcpy(info.buffer.addr(), buf.addr(), buf.size());
	return true;
}

void GBuffer::set(Buffer buf) {
	if (copy(buf))
		flush();
}