#include "types/buffer.h"
#include "graphics/objects/gbuffer.h"
using namespace oi::gc;
using namespace oi;

GBuffer::GBuffer(GBufferInfo info) : info(info) {}
GBufferType GBuffer::getType() { return info.type; }
u32 GBuffer::getSize() { return info.size; }
u8 *GBuffer::getAddress() { return info.ptr; }

GBufferExt &GBuffer::getExtension() { return ext; }
const GBufferInfo GBuffer::getInfo() { return info; }

bool GBuffer::set(Buffer buf) {
	open();
	bool b = copy(buf);
	close();
	return b;
}

bool GBuffer::copy(Buffer buf) {

	if (info.size == 0 || info.ptr == nullptr)
		return Log::error("GBuffer::copy; can't copy to a null buffer. Please unmap first.");

	if (buf.size() > info.size)
		return Log::error("GBuffer::set please use a buffer that matches the gbuffer's size");

	memcpy(info.ptr, buf.addr(), buf.size());
	return true;
}