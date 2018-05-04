#include "graphics/rendertarget.h"
#include <utils/log.h>
using namespace oi::gc;
using namespace oi;

u32 RenderTarget::getVersions() { return info.buffering; }
u32 RenderTarget::getTargets() { return info.targets; }

Texture *RenderTarget::getDepthBuffer() { return info.depth; }
Texture *RenderTarget::getTarget(u32 target, u32 version) {

	if (target > info.targets || version >= info.buffering)
		return (Texture*) Log::warn("Target and/or version out of range; please check getVersions() and getTargets()");

	if (target == 0)
		return info.depth;

	return info.textures[(target - 1) * info.buffering + version];
}

Vec2u RenderTarget::getSize() { return info.res; }
RenderTargetExt &RenderTarget::getExtension() { return ext; }

RenderTarget::RenderTarget(RenderTargetInfo info) : info(info){}