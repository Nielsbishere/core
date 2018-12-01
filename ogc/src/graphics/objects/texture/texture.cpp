#include "graphics/objects/texture/texture.h"
using namespace oi::gc;
using namespace oi;

TextureFormat Texture::getFormat() { return info.format; }
TextureUsage Texture::getUsage() { return info.usage; }
Vec2u Texture::getSize() { return info.res; }
bool Texture::isOwned() { return owned; }
TextureHandle Texture::getHandle() { return info.handle; }

Texture::Texture(TextureInfo info) : info(info) {}

TextureExt &Texture::getExtension() { return ext; }
const TextureInfo Texture::getInfo() { return info; }

void Texture::initParent(TextureList *parent) {
	if (info.parent == nullptr)
		info.parent = parent;
}

void Texture::flush() {
	info.isChanged = true;
}

bool Texture::shouldStage() {
	return info.isChanged;
}