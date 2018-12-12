#include "graphics/graphics.h"
#include "graphics/objects/texture/versionedtexture.h"
using namespace oi::gc;
using namespace oi;

bool Graphics::isDepthFormat(TextureFormat format) {
	return format.getValue() >= TextureFormat::D16 && format.getValue() <= TextureFormat::Depth;
}

bool Graphics::hasStencil(TextureFormat format) {
	return format.getName().contains("S");
}

TextureLoadFormat Graphics::getLoadFormat(TextureFormat format) {

	if (format.getValue() >= TextureFormat::BGRA8)
		return format.getName().replace("BGR", "RGB");

	if (isDepthFormat(format))
		return format.getName().replace("D", "R");

	return format.getName();
}

u32 Graphics::getChannelSize(TextureFormat format) {

	const TextureFormat_s &val = format.getValue();

	if (val == TextureFormat::Undefined || val == TextureFormat::Depth)
		return 0U;

	if (val < TextureFormat::RGBA16) return 1U;
	if (val < TextureFormat::RGBA32f || val == TextureFormat::D16) return 2U;
	if (val == TextureFormat::D16S8) return 3U;
	if (val < TextureFormat::RGBA64f || val == TextureFormat::D32 || val == TextureFormat::D24S8) return 4U;
	if (val < TextureFormat::D16) return 8U;
	if (val == TextureFormat::D32S8) return 5U;

	return 1U;
}

u32 Graphics::getChannels(TextureFormat format) {

	u32 val = format.getValue();

	if (val == 0) return 0U;
	if (val < TextureFormat::D16) return 4U - (val - 1U) % 4U;
	if (val < TextureFormat::sRGBA8) return 1U;
	if(val < TextureFormat::BGRA8) return 4U - (val - TextureFormat::sRGBA8);

	return 4 - (val - TextureFormat::BGRA8) % 2U;
}

TextureFormatStorage Graphics::getFormatStorage(TextureFormat format) {

	if (format.getName().endsWith("u")) return TextureFormatStorage::UINT;
	if (format.getName().endsWith("i")) return TextureFormatStorage::INT;

	return format.getName().endsWith("64f") ? TextureFormatStorage::DOUBLE : TextureFormatStorage::FLOAT;
}

bool Graphics::isCompatible(TextureFormat a, TextureFormat b) {
	return getFormatStorage(a) == getFormatStorage(b) && getChannels(a) == getChannels(b);
}


Vec4d Graphics::convertColor(Vec4d cl, TextureFormat format) {

	Vec4d color = Vec4d(0.0);
	u32 colors = getChannels(format);

	if (colors == 0U) return color;

	memcpy(color.arr, cl.arr, sizeof(f64) * colors);

	return color;

}

u32 Graphics::getFormatSize(TextureFormat format) { return getChannelSize(format) * getChannels(format); }
GraphicsExt &Graphics::getExtension() { return ext; }

RenderTarget *Graphics::getBackBuffer() { return backBuffer; }
u32 Graphics::getBuffering() { return buffering; }

void Graphics::printObjects() {
	for (auto a : objects)
		for (auto b : a.second)
			Log::println(b->getName() + " (" + b->getTypeName() + ") refCount " + b->refCount);
}

bool Graphics::remove(GraphicsObject *go) {

	size_t id = go->getHash();

	auto it = objects.find(id);
	if (it == objects.end()) return false;

	auto &vec = it->second;

	auto itt = std::find(vec.begin(), vec.end(), go);
	if (itt == vec.end()) return false;

	vec.erase(itt);
	return true;
}

bool Graphics::contains(GraphicsObject *go) const {

	size_t id = go->getHash();

	auto it = objects.find(id);
	if (it == objects.end()) return false;

	auto &vec = it->second;

	auto itt = std::find(vec.begin(), vec.end(), go);
	if (itt == vec.end()) return false;

	return true;

}

bool Graphics::destroyObject(GraphicsObject *go) {

	if (go == nullptr) return false;

	auto it = objects.find(go->hash);
	if (it == objects.end()) return false;

	auto &vec = it->second;

	auto itt = std::find(vec.begin(), vec.end(), go);
	if (itt == vec.end()) return false;

	if (--go->refCount <= 0) {
		vec.erase(itt);
		allocator.dealloc(go);
	}

	return true;

}

void Graphics::use(GraphicsObject *go) {
	if (contains(go)) ++go->refCount;
}