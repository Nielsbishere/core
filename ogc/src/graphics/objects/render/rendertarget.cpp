#include "utils/log.h"
#include "graphics/graphics.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;


u32 RenderTarget::getVersions() { return g->getBuffering(); }
u32 RenderTarget::getTargets() { return info.targets; }

Texture *RenderTarget::getDepth() { return info.depth; }

VersionedTexture *RenderTarget::getTarget(u32 target) {

	if (target >= getTargets())
		return (VersionedTexture*) Log::error("Target out of range; please check getTargets()");

	return info.textures[target];
}

Vec2u RenderTarget::getSize() { return info.res; }
bool RenderTarget::isOwned() { return owned; }
bool RenderTarget::isComputeTarget() { return info.isComputeTarget; }

const RenderTargetInfo &RenderTarget::getInfo() { return info; }

RenderTarget::RenderTarget(RenderTargetInfo info) : info(info){}
RenderTarget::~RenderTarget() {

	for (VersionedTexture *t : info.textures)
		g->destroy(t);

	g->destroy(info.depth);

	destroyData();

}

bool RenderTarget::init(bool isOwned) {

	owned = isOwned;

	if (!owned)
		return initData();

	info.depth = info.depthFormat == TextureFormat::Undefined ? nullptr : g->create(getName() + " depth", TextureInfo(info.res, info.depthFormat, TextureUsage::Render_depth));

	std::vector<VersionedTexture*> &textures = info.textures;
	textures.resize(info.targets);

	std::vector<Texture*> vtextures(g->getBuffering());

	for (u32 i = 0; i < info.targets; ++i) {

		TextureInfo texInfo = TextureInfo(info.res, info.formats[i], info.isComputeTarget ? TextureUsage::Compute_target : TextureUsage::Render_target);

		for (u32 j = 0; j < g->getBuffering(); ++j)
			g->use(vtextures[j] = g->create(getName() + " " + i + " #" + j, texInfo));

		g->use(textures[i] = g->create(getName() + " " + i, VersionedTextureInfo(vtextures)));

	}

	return initData();

}