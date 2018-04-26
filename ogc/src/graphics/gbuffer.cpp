#include "graphics/gbuffer.h"
using namespace oi::gc;
using namespace oi;

GBuffer::GBuffer(GBufferInfo info) : info(info) {}
GBufferType GBuffer::getType() { return info.type; }
u32 GBuffer::getSize() { return info.size; }
GBufferExt &GBuffer::getExtension() { return ext; }