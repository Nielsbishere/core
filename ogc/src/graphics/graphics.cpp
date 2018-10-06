#include <file/filemanager.h>
#include "graphics/graphicsobject.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "graphics/rendertarget.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/format/oish.h"
#include "graphics/pipeline.h"
#include "graphics/pipelinestate.h"
#include "graphics/gbuffer.h"
#include "graphics/shaderbuffer.h"
#include "graphics/sampler.h"
#include "graphics/camera.h"
#include "graphics/mesh.h"
#include "graphics/drawlist.h"
#include "graphics/versionedtexture.h"
#include "graphics/texturelist.h"
#include "graphics/materiallist.h"
#include "api/stbi/stbi_load.h"

#undef min
#undef max

using namespace oi::gc;
using namespace oi;

bool Graphics::isDepthFormat(TextureFormat format) {
	return format.getValue() >= TextureFormat::D16 && format.getValue() <= TextureFormat::Depth;
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

	u32 bits = getChannelSize(format);
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

Shader *Graphics::create(String name, ShaderInfo info) {

	if (info.stages.size() == 0) {

		SHFile file;

		if (!oiSH::read(info.path, file))
			return (Shader*)Log::throwError<Graphics, 0x1A>("Couldn't read shader");

		String path = info.path;

		info = oiSH::convert(this, file);

	}

	Shader *s = init<Shader>(name, info);

	for (ShaderStage *ss : s->getInfo().stage)
		++ss->refCount;

	return s;
}

ShaderStage *Graphics::create(String name, ShaderStageInfo info) {
	info.code = Buffer(info.code.addr(), info.code.size());
	return init<ShaderStage>(name, info);
}


Texture *Graphics::create(String name, TextureInfo info) {

	if (info.path != "") {

		//Set up a buffer to load

		if (info.loadFormat == TextureLoadFormat::Undefined)
			return (Texture*) Log::throwError<Graphics, 0x1C>("Couldn't load texture; Texture load format is invalid");

		if (!wc::FileManager::get()->read(info.path, info.dat))										//Temporarily store the file data into info.dat
			return (Texture*)Log::throwError<Graphics, 0x1B>("Couldn't load texture from disk");

		int width, height, comp;

		int perChannel = (int) (info.loadFormat.getValue() - 1) % 4 + 1;

		//Convert data to image info

		u8 *ptr = (u8*) stbi_load_from_memory((const stbi_uc*) info.dat.addr(), (int) info.dat.size(), &width, &height, &comp, perChannel);
		stbi_image_free(info.dat.addr());
		info.dat = Buffer::construct(ptr, (u32) perChannel * width * height);

		info.res = { (u32) width, (u32) height };

		info.mipLevels = (u32) std::floor(std::log2(std::max(info.res.x, info.res.y))) + 1U;
	} else 
		info.mipLevels = 1U;

	return init<Texture>(name, info);
}

RenderTarget *Graphics::create(String name, RenderTargetInfo info) {

	info.depth = info.depthFormat == TextureFormat::Undefined ? nullptr : create(name + " depth", TextureInfo(info.res, info.depthFormat, TextureUsage::Render_depth));

	std::vector<VersionedTexture*> &textures = info.textures;
	std::vector<Texture*> vtextures(buffering);
	textures.resize(info.targets);

	for (u32 i = 0; i < info.targets; ++i) {

		TextureInfo texInfo = TextureInfo(info.res, info.formats[i], TextureUsage::Render_target);

		for (u32 j = 0; j < buffering; ++j)
			++(vtextures[j] = create(name + " target " + i + " version " + j, texInfo))->refCount;

		++(textures[i] = create(name + " target " + i, VersionedTextureInfo(vtextures)))->refCount;

	}

	return init<RenderTarget>(name, info);
}

Pipeline *Graphics::create(String name, PipelineInfo info) {

	++info.shader->refCount;

	if(info.renderTarget != nullptr)
		++info.renderTarget->refCount;

	if (info.pipelineState != nullptr)
		++info.pipelineState->refCount;

	return init<Pipeline>(name, info);
}

PipelineState *Graphics::create(String name, PipelineStateInfo info) {
	return init<PipelineState>(name, info);
}

GBuffer *Graphics::create(String name, GBufferInfo info) {
	return init<GBuffer>(name, info);
}

ShaderBuffer *Graphics::create(String name, ShaderBufferInfo info) {
	return init<ShaderBuffer>(name, info);
}

Sampler *Graphics::create(String name, SamplerInfo info) {
	return init<Sampler>(name, info);
}

Camera *Graphics::create(String name, CameraInfo info) {
	return init<Camera>(name, info);
}

MeshBuffer *Graphics::create(String name, MeshBufferInfo info) {
	return init<MeshBuffer>(name, info);
}

Mesh *Graphics::create(String name, MeshInfo info) {
	return init<Mesh>(name, info);
}

DrawList *Graphics::create(String name, DrawListInfo info) {
	return init<DrawList>(name, info);
}

VersionedTexture *Graphics::create(String name, VersionedTextureInfo info) {
	return init<VersionedTexture>(name, info);
}

TextureList *Graphics::create(String name, TextureListInfo info) {
	return init<TextureList>(name, info);
}

MaterialList *Graphics::create(String name, MaterialListInfo info) {
	return init<MaterialList>(name, info);
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

bool Graphics::destroy(GraphicsObject *go) {

	if (go == nullptr) return false;

	size_t id = go->getHash();

	auto it = objects.find(id);
	if (it == objects.end()) return false;

	auto &vec = it->second;

	auto itt = std::find(vec.begin(), vec.end(), go);
	if (itt == vec.end()) return false;

	if(--go->refCount <= 0)
		delete go;
	
	return true;
}

void Graphics::use(GraphicsObject *go) {
	if (contains(go)) ++go->refCount;
}