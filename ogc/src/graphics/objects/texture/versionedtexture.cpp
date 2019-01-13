#include "graphics/graphics.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

TextureFormat VersionedTexture::getFormat() const { return getVersion(0)->getInfo().format; }
TextureUsage VersionedTexture::getUsage() const { return getVersion(0)->getInfo().usage; }
Vec2u VersionedTexture::getSize() const { return getVersion(0)->getInfo().res; }
bool VersionedTexture::isOwned() const { return getVersion(0)->isOwned(); }
u32 VersionedTexture::getVersions() const { return info.versions; }
const VersionedTextureInfo &VersionedTexture::getInfo() const { return info; }
VersionedTexture::VersionedTexture(VersionedTextureInfo info) : info(info) { }
bool VersionedTexture::init() { return getVersions() != 0; }

Texture *VersionedTexture::getVersion(u32 i) const {

	if (i >= getVersions())
		return nullptr;

	return info.version[i];

}

VersionedTexture::~VersionedTexture() {

	for (Texture *tex : info.version)
		g->destroy(tex);

}

void VersionedTexture::resize(Vec2u size) {

	for (Texture *tex : info.version)
		tex->resize(size);

}