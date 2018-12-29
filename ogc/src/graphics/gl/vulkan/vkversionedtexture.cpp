#ifdef __VULKAN__
#include "graphics/graphics.h"
#include "graphics/gl/vulkan.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

struct VkVersionedTexture;

bool VersionedTexture::getPixels(Vec2u start, Vec2u length, CopyBuffer &output) {
	GraphicsExt &graphics = g->getExtension();
	return info.version[graphics.current % info.versions]->getPixels(start, length, output);
}

bool VersionedTexture::write(String path, Vec2u start, Vec2u length) {
	GraphicsExt &graphics = g->getExtension();
	return info.version[graphics.current % info.versions]->write(path, start, length);
}

#endif