#include "file/filemanager.h"
#include "graphics/graphics.h"
#include "graphics/objects/texture/texture.h"
#include "graphics/objects/texture/texturelist.h"

#pragma warning(push)
#pragma warning(disable: 4100)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef __WINDOWS__
#define STBI_MSC_SECURE_CRT
#endif

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#pragma warning(pop)

#undef min
#undef max

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

TextureInfo::TextureInfo(Vec2u res, TextureFormat format, TextureUsage usage) :
	parent(nullptr), res(res), format(format), usage(usage), mipFilter(TextureMipFilter::None), loadFormat(Graphics::getLoadFormat(format)) {}

TextureInfo::TextureInfo(TextureList *parent, String path, TextureLoadFormat loadFormat, TextureMipFilter mipFilter) : 
	parent(parent), path(path), usage(TextureUsage::Image), loadFormat(loadFormat), format(loadFormat.getName()), mipFilter(mipFilter) {}

TextureInfo::TextureInfo(TextureList *parent, Vec2u res, TextureLoadFormat format, TextureMipFilter mipFilter) : 
	parent(parent), res(res), path(""), usage(TextureUsage::Image), loadFormat(format), format(format.getName()), mipFilter(mipFilter) {}

TextureFormat Texture::getFormat() { return info.format; }
TextureUsage Texture::getUsage() { return info.usage; }
Vec2u Texture::getSize() { return info.res; }
u32 Texture::getStride() { return Graphics::getFormatSize(info.format); }
u32 Texture::getCpuSize() { return getStride() * info.res.x * info.res.y; }
bool Texture::isOwned() { return owned; }
TextureHandle Texture::getHandle() { return info.handle; }

Texture::Texture(TextureInfo info) : info(info) {}

Texture::~Texture() {

	info.dat.deconstruct();

	if (info.parent != nullptr) {
		info.parent->dealloc(this);
		g->destroy(info.parent);
	}

	destroyData(false);
}

const TextureInfo Texture::getInfo() { return info; }

void Texture::initParent(TextureList *parent) {
	if (info.parent == nullptr) {
		info.parent = parent;
		info.handle = parent->alloc(this);
		g->use(parent);
	}
}

bool Texture::setPixels(Vec2u start, Vec2u length, Buffer values) {

	if (info.dat.size() == 0)
		return Log::throwError<Texture, 0x5>("Texture::setPixels can only be applied to loaded textures");

	if (start.x + length.x > info.res.x || start.y + length.y > info.res.y)
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
		return getPixelsGpu(start, length, output);

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

bool Texture::write(String path, Vec2u start, Vec2u length) {

	if (length == Vec2u())
		length = info.res;

	if (info.loadFormat == TextureLoadFormat::Undefined)
		return Log::throwError<Texture, 0xB>("Texture::write can only be applied to loaded textures");

	if(path.getExtension() != "png")
		return Log::throwError<Texture, 0xF>("Texture::write currently only accepts png files");

	CopyBuffer pixels;

	if(info.dat.size() != 0 && !getPixels(start, length, pixels))
		return Log::throwError<Texture, 0xC>("Texture::write couldn't read pixels");
	else if(!getPixelsGpu(start, length, pixels))
		return Log::throwError<Texture, 0x7>("Texture::write couldn't read (GPU) pixels");

	int perChannel = (int)(info.loadFormat.getValue() - 1) % 4 + 1;

	int pngLength = 0;
	u8 *png = stbi_write_png_to_mem(pixels.addr(), 0, (int)length.x, (int)length.y, perChannel, &pngLength);

	if(png == nullptr)
		return Log::throwError<Texture, 0xE>("Texture::write couldn't write texture to png");

	bool written = FileManager::get()->write(path, Buffer::construct(png, (u32) pngLength));
	if (!written)
		Log::error(String("Texture::write couldn't write to output path ") + path);

	free(png);
	return written;
}

bool Texture::read(String path, Vec2u start, Vec2u length) {

	if (info.loadFormat == TextureLoadFormat::Undefined || info.dat.size() == 0)
		return Log::throwError<Texture, 0x9>("Texture::read can only be applied to loaded textures");

	Buffer temp;

	if (!FileManager::get()->read(path, temp))
		return (Texture*) Log::error("Couldn't load texture from disk");

	int width, height, comp;
	u32 perChannel = (info.loadFormat.getValue() - 1) % 4 + 1;

	u8 *ptr = (u8*)stbi_load_from_memory((const stbi_uc*)temp.addr(), (int)temp.size(), &width, &height, &comp, (int) perChannel);

	if (ptr == nullptr)
		return Log::throwError<Texture, 0xD>("Texture::read couldn't read data from file");

	temp.deconstruct();

	if (length.x == 0)
		length.x = (u32)width;

	if (length.y == 0)
		length.y = (u32)height;

	Vec2u loadedSize((u32)width, (u32)height);

	if (length != loadedSize) {

		Buffer copy;

		if (length.x == (u32)width)
			copy = Buffer(ptr, length.y * length.x * perChannel);
		else {
			copy = Buffer(length.y * length.x * perChannel);

			for(u32 j = 0; j < length.y; ++j)
				memcpy(copy.addr() + j * length.x * perChannel, ptr + j * (u32) width * perChannel, (u32) width * perChannel);
		}

		free(ptr);

		if (!setPixels(start, length, copy))
			return Log::throwError<Texture, 0xA>("Texture::read couldn't copy pixels into texture");

		copy.deconstruct();
		return true;
	}

	if(!setPixels(start, length, Buffer::construct(ptr, (u32) width * height * perChannel)))
		return Log::throwError<Texture, 0xA>("Texture::read couldn't copy pixels into texture");

	free(ptr);
	return true;
}

void Texture::resize(Vec2u size) {

	if (size.x == 0 || size.y == 0)
		Log::throwError<Texture, 0x13>("Resizing to 0,0 is illegal, that resolution is only allowed to reserve a texture handle");

	if (info.usage != TextureUsage::Render_depth && info.usage != TextureUsage::Render_target && info.usage != TextureUsage::Compute_target)
		Log::throwError<Texture, 0x14>("Resizing a non-target texture is illegal, the creation size is constant");

	info.res = size;
	destroyData(true);
	initData();

}

bool Texture::init(bool isOwned) {

	owned = isOwned;

	int perChannel = (int)(info.loadFormat.getValue() - 1) % 4 + 1;

	if (info.mipFilter != TextureMipFilter::None) {

		//Set up a buffer to load

		if (info.loadFormat == TextureLoadFormat::Undefined)
			return Log::throwError<Texture, 0x10>("Couldn't load texture; Texture load format is invalid");

		if (!wc::FileManager::get()->read(info.path, info.dat))										//Temporarily store the file data into info.dat
			return Log::throwError<Texture, 0x11>("Couldn't load texture from disk");

		int width, height, comp;

		//Convert data to image info

		u8 *ptr = (u8*)stbi_load_from_memory((const stbi_uc*)info.dat.addr(), (int)info.dat.size(), &width, &height, &comp, perChannel);
		info.dat.deconstruct();
		info.dat = Buffer(ptr, (u32)perChannel * width * height);
		stbi_image_free(ptr);

		info.res = { (u32)width, (u32)height };
		info.mipLevels = info.mipFilter == TextureMipFilter::None ? 1U : (u32)std::floor(std::log2(std::max(info.res.x, info.res.y))) + 1U;

	} else
		info.mipLevels = 1U;

	if (info.dat.size() == 0 && info.usage == TextureUsage::Image) {

		if (info.loadFormat == TextureLoadFormat::Undefined)
			return Log::throwError<Texture, 0x12>("Couldn't create CPU buffer for texture; invalid format");

		info.dat = Buffer((u32)perChannel * info.res.x * info.res.y);

	}

	if ((info.res.x == 0 || info.res.y == 0) && info.usage != TextureUsage::Render_depth && info.usage != TextureUsage::Render_target && info.usage != TextureUsage::Compute_target)
		Log::throwError<Texture, 0x15>("Initializing with resolution 0,0 isn't allowed, because non-target textures cannot be resized");

	return initData();
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