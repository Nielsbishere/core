#include "graphics/objects/texture/texture.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

TextureFormat Texture::getFormat() { return info.format; }
TextureUsage Texture::getUsage() { return info.usage; }
Vec2u Texture::getSize() { return info.res; }
u32 Texture::getStride() { return Graphics::getFormatSize(info.format); }
u32 Texture::getCpuSize() { return getStride() * info.res.x * info.res.y; }
bool Texture::isOwned() { return owned; }
TextureHandle Texture::getHandle() { return info.handle; }

Texture::Texture(TextureInfo info) : info(info) {}

TextureExt &Texture::getExtension() { return ext; }
const TextureInfo Texture::getInfo() { return info; }

void Texture::initParent(TextureList *parent) {
	if (info.parent == nullptr)
		info.parent = parent;
}

bool Texture::setPixels(Vec2u start, Vec2u length, Buffer values) {

	if (info.dat.size() == 0)
		return Log::throwError<Texture, 0x5>("Texture::setPixels can only be applied to loaded textures");

	if (start.x + length.x >= info.res.x || start.y + length.y >= info.res.y)
		return Log::throwError<Texture, 0x3>("Texture::setPixels was out of bounds");

	u32 stride = getStride();

	if (values.size() < stride * length.x * length.y)
		return Log::throwError<Texture, 0x4>("Texture::setPixels invalid format");

	memcpy(info.dat.addr() + start.y * info.res.x * stride + start.x * stride, values.addr(), length.x * length.y * stride);
	flush(start, length);
	return true;
}

bool Texture::shouldStage() {
	return info.changedEnd.x != 0 && info.changedEnd.y != 0;
}

void Texture::flush(Vec2u start, Vec2u length) {

	if (start.x + length.x > info.res.x || start.y + length.y > info.res.y)
		Log::throwError<Texture, 0x5>("Texture::flush out of bounds");

	if (start.x < info.changedStart.x)
		info.changedStart.x = start.x;

	if (start.y < info.changedStart.y)
		info.changedStart.y = start.y;

	if (start.x + length.x > info.changedEnd.x)
		info.changedEnd.x = start.x + length.x;

	if (start.y + length.y > info.changedEnd.y)
		info.changedEnd.y = start.y + length.y;

}