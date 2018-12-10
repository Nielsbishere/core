#include "graphics/objects/texture/texture.h"
#include "graphics/graphics.h"
#include "file/filemanager.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#pragma warning(pop)

using namespace oi::gc;
using namespace oi::wc;
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

	if (values.size() < length.y * length.x * stride)
		return Log::throwError<Texture, 0x4>("Texture::setPixels invalid format");

	if (length == info.res)
		memcpy(info.dat.addr(), values.addr(), length.x * length.y * stride);
	else if (length.x == info.res.x)
		memcpy(info.dat.addr() + start.y * info.res.x * stride, values.addr(), length.y * info.res.x * stride);
	else {
		for (u32 j = 0; j < length.y; ++j)
			memcpy(info.dat.addr() + ((start.y + j) * info.res.x + start.x) * stride, values.addr() + j * length.x * stride, length.x * stride);
	}

	flush(start, length);
	return true;
}


bool Texture::getPixels(Vec2u start, Vec2u length, CopyBuffer &output) {

	if (info.dat.size() == 0)
		return Log::throwError<Texture, 0x7>("Texture::getPixels can only be applied to loaded textures");

	if (start.x + length.x >= info.res.x || start.y + length.y >= info.res.y)
		return Log::throwError<Texture, 0x8>("Texture::getPixels was out of bounds");

	u32 stride = getStride();

	if (length == info.res)
		output = info.dat;
	else {

		output = CopyBuffer(length.y * length.x * stride);

		if (info.res.x == length.x)
			memcpy(output.addr(), info.dat.addr() + start.y * info.res.x * stride, length.y * info.res.x * stride);
		else {
			for (u32 j = 0; j < length.y; ++j)
				memcpy(output.addr() + j * length.x * stride, info.dat.addr() + ((start.y + j) * info.res.x + start.x) * stride, length.x * stride);
		}

	}

	return true;

}

//void Texture::write(String path, Vec2u start, Vec2u length) {
//
//}

bool Texture::read(String path, Vec2u start, Vec2u length) {

	if (info.dat.size() == 0)
		return Log::throwError<Texture, 0x9>("Texture::read can only be applied to loaded textures");

	Buffer temp;

	if (!wc::FileManager::get()->read(info.path, temp))
		return (Texture*) Log::error("Couldn't load texture from disk");

	int width, height, comp;
	int perChannel = (int)(info.loadFormat.getValue() - 1) % 4 + 1;

	u8 *ptr = (u8*)stbi_load_from_memory((const stbi_uc*)temp.addr(), (int)temp.size(), &width, &height, &comp, perChannel);
	temp.deconstruct();

	if (length.x == 0)
		length.x = (u32)width;

	if (length.y == 0)
		length.y = (u32)height;

	if(!setPixels(start, length, Buffer::construct(ptr, (u32) width * height * perChannel)))
		return Log::throwError<Texture, 0xA>("Texture::read couldn't copy pixels into texture");

	free(ptr);
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