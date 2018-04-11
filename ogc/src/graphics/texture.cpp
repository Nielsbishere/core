#include "graphics/texture.h"
using namespace oi::gc;
using namespace oi;

TextureFormat Texture::getFormat() { return format; }
TextureUsage Texture::getUsage() { return usage; }
Vec2u Texture::getSize() { return size; }

Texture::Texture(Vec2u size, TextureFormat format, TextureUsage usage) : size(size), format(format), usage(usage) {}