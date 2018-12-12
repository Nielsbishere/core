#ifdef __VULKAN__
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

struct VkVersionedTexture;

bool VersionedTexture::getPixels(Vec2u start, Vec2u length, CopyBuffer &output) {
	VkGraphics &graphics = g->getExtension();
	return info.version[graphics.current % info.versions]->getPixels(start, length, output);
}

bool VersionedTexture::write(String path, Vec2u start, Vec2u length) {
	VkGraphics &graphics = g->getExtension();
	return info.version[graphics.current % info.versions]->write(path, start, length);
}

#endif