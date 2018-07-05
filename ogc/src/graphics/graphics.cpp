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

Shader *Graphics::create(ShaderInfo info) {

	SHFile file;

	if (!oiSH::read(info.path, file))
		return (Shader*)Log::throwError<Graphics, 0x1A>("Couldn't read shader");

	String path = info.path;

	info = oiSH::convert(this, file);

	Shader *s = init<Shader>(info);

	for (ShaderStage *ss : s->getInfo().stage)
		++ss->refCount;

	return s;
}

ShaderStage *Graphics::create(ShaderStageInfo info) {
	info.code = Buffer(info.code.addr(), info.code.size());
	return init<ShaderStage>(info);
}


Texture *Graphics::create(TextureInfo info) {

	if (info.path != "") {

		//Set up a buffer to load

		if (info.loadFormat == TextureLoadFormat::Undefined)
			return (Texture*) Log::throwError<Graphics, 0x1C>("Couldn't load texture; Texture load format is invalid");

		if (!wc::FileManager::get()->read(info.path, info.dat))										//Temporarily store the file data into info.dat
			return (Texture*)Log::throwError<Graphics, 0x1B>("Couldn't load texture from disk");

		int width, height, comp;

		int perChannel = (int) info.loadFormat.getValue();

		//Convert data to image info

		u8 *ptr = (u8*) stbi_load_from_memory((const stbi_uc*) info.dat.addr(), (int) info.dat.size(), &width, &height, &comp, perChannel);
		stbi_image_free(info.dat.addr());
		info.dat = Buffer::construct(ptr, (u32) perChannel * width * height);

		info.res = { (u32) width, (u32) height };

		info.mipLevels = (u32) std::floor(std::log2(std::max(info.res.x, info.res.y))) + 1U;
	} else 
		info.mipLevels = 1U;

	return init<Texture>(info);
}

RenderTarget *Graphics::create(RenderTargetInfo info) {

	info.depth = info.depthFormat == TextureFormat::Undefined ? nullptr : create(TextureInfo(info.res, info.depthFormat, TextureUsage::Render_depth));

	std::vector<Texture*> &textures = info.textures;
	textures.resize(info.buffering * info.targets);

	for (u32 i = 0; i < (u32) textures.size(); ++i)
		textures[i] = create(TextureInfo(info.res, info.formats[i / buffering], TextureUsage::Render_target));

	for (Texture *t : textures)
		++t->refCount;

	return init<RenderTarget>(info);
}

Pipeline *Graphics::create(PipelineInfo info) {

	++info.shader->refCount;

	if(info.renderTarget != nullptr)
		++info.renderTarget->refCount;

	if (info.pipelineState != nullptr)
		++info.pipelineState->refCount;

	return init<Pipeline>(info);
}

PipelineState *Graphics::create(PipelineStateInfo info) {
	return init<PipelineState>(info);
}

GBuffer *Graphics::create(GBufferInfo info) {
	return init<GBuffer>(info);
}

ShaderBuffer *Graphics::create(ShaderBufferInfo info) {
	return init<ShaderBuffer>(info);
}

Sampler *Graphics::create(SamplerInfo info) {
	return init<Sampler>(info);
}

Camera *Graphics::create(CameraInfo info) {
	return init<Camera>(info);
}

MeshBuffer *Graphics::create(MeshBufferInfo info) {
	return init<MeshBuffer>(info);
}

Mesh *Graphics::create(MeshInfo info) {
	return init<Mesh>(info);
}

DrawList *Graphics::create(DrawListInfo info) {
	return init<DrawList>(info);
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