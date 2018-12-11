#include "file/filemanager.h"
#include "graphics/graphics.h"
#include "graphics/format/oish.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/render/rendertarget.h"
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
			return (Shader*)Log::throwError<Graphics, 0x1>("Couldn't read shader");

		String path = info.path;

		info = oiSH::convert(this, file);

	} else {
	
		for (ShaderStageInfo &inf : info.stages) {
			if (inf.type == ShaderStageType::Vertex_shader)
				info.inputs = inf.input;
			else if (inf.type == ShaderStageType::Fragment_shader)
				info.outputs = inf.output;
		}

		if (info.stage.size() == 0) {

			info.stage.resize(info.stages.size());

			for (u32 i = 0, j = (u32)info.stages.size(); i < j; ++i)
				info.stage[i] = create(info.path + " " + info.stages[i].type.getName(), info.stages[i]);

		}
	
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

RenderTarget *Graphics::create(String name, RenderTargetInfo info) {

	info.depth = info.depthFormat == TextureFormat::Undefined ? nullptr : create(name + " depth", TextureInfo(info.res, info.depthFormat, TextureUsage::Render_depth));

	std::vector<VersionedTexture*> &textures = info.textures;
	std::vector<Texture*> vtextures(buffering);
	textures.resize(info.targets);

	for (u32 i = 0; i < info.targets; ++i) {

		TextureInfo texInfo = TextureInfo(info.res, info.formats[i], TextureUsage::Render_target);

		for (u32 j = 0; j < buffering; ++j)
			++(vtextures[j] = create(name + " - " + i + " #" + j, texInfo))->refCount;

		++(textures[i] = create(name + " - " + i, VersionedTextureInfo(vtextures)))->refCount;

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