#include "graphics/gbuffer.h"
#include <types/buffer.h>
using namespace oi::gc;
using namespace oi;

GBuffer::GBuffer(GBufferInfo info) : info(info) {}
GBufferType GBuffer::getType() { return info.type; }
u32 GBuffer::getSize() { return info.size; }
GBufferExt &GBuffer::getExtension() { return ext; }
u8 *GBuffer::getAddress() { return info.ptr; }

bool GBuffer::set(Buffer buf) {
	open();
	bool b = copy(buf);
	close();
	return b;
}

bool GBuffer::copy(Buffer buf) {

	if (buf.size() != info.size)
		return Log::error("GBuffer::set please use a buffer that matches the gbuffer's size");

	memcpy(info.ptr, buf.addr(), info.size);
	return true;
}