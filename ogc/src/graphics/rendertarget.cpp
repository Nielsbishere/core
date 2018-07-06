#include "graphics/rendertarget.h"
#include <utils/log.h>
using namespace oi::gc;
using namespace oi;


u32 RenderTarget::getVersions() { return info.buffering; }
u32 RenderTarget::getTargets() { return info.targets; }

Texture *RenderTarget::getDepth() { return info.depth; }

VersionedTexture *RenderTarget::getTarget(u32 target) {

	if (target >= getTargets())
		return (VersionedTexture*) Log::error("Target out of range; please check target()");

	return info.textures[target];
}

Vec2u RenderTarget::getSize() { return info.res; }

RenderTargetExt &RenderTarget::getExtension() { return ext; }
const RenderTargetInfo RenderTarget::getInfo() { return info; }

RenderTarget::RenderTarget(RenderTargetInfo info) : info(info){}