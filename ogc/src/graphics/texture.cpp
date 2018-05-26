#include "graphics/texture.h"
using namespace oi::gc;
using namespace oi;

TextureFormat Texture::getFormat() { return info.format; }
TextureUsage Texture::getUsage() { return info.usage; }
Vec2u Texture::getSize() { return info.res; }
bool Texture::isOwned() { return owned; }

Texture::Texture(TextureInfo info) : info(info) {}

TextureExt &Texture::getExtension() { return ext; }
const TextureInfo Texture::getInfo() { return info; }