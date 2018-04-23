#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "graphics/rendertarget.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/format/oish.h"
#include "graphics/pipeline.h"
#include "graphics/pipelinestate.h"
using namespace oi::gc;
using namespace oi;

bool Graphics::isDepthFormat(TextureFormat format) {
	return format.getValue() >= TextureFormat::D16 && format.getValue() <= TextureFormat::Depth;
}

u32 Graphics::getChannelSize(TextureFormat format) {

	const TextureFormat_s &val = format.getValue();

	if (val == TextureFormat::Undefined || val == TextureFormat::Depth)
		return 0U;

	if (val < TextureFormat::RGBA16 || val == TextureFormat::D16) return 1U;
	if (val < TextureFormat::RGBA32f) return 2U;
	if (val == TextureFormat::D16S8) return 3U;
	if (val < TextureFormat::D16 || val == TextureFormat::D32 || val == TextureFormat::D24S8) return 4U;
	if (val == TextureFormat::D32S8) return 5U;

	return 4U;
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

	return TextureFormatStorage::FLOAT;
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

	if (!oiSH::read(this, info.path, info))
		return (Shader*)Log::throwError<Graphics, 0x1A>("Couldn't read shader");

	Shader *s = new Shader(info);
	if (!s->init(this))
		return (Shader*)Log::throwError<Graphics, 0x1B>("Couldn't initialize shader");

	return s;
}

ShaderStage *Graphics::create(ShaderStageInfo info) {

	info.code = Buffer(info.code.addr(), info.code.size());

	ShaderStage *ss = new ShaderStage(info);
	if (!ss->init(this))
		return (ShaderStage*)Log::throwError<Graphics, 0x1C>("Couldn't initialize shader stage");

	return ss;
}

bool Graphics::cleanCommandList(CommandList *cmd) {

	for (auto it = commandList.begin(); it != commandList.end(); ++it)
		if (*it == cmd) {
			commandList.erase(it);
			return true;
		}

	return false;

}


Texture *Graphics::create(TextureInfo info) {

	Texture *tex = new Texture(info);

	if (!tex->init(this))
		return (Texture*)Log::throwError<Graphics, 0xB>("Couldn't create texture");

	return tex;
}

RenderTarget *Graphics::create(RenderTargetInfo info) {

	Texture *depth = info.depthFormat == TextureFormat::Undefined ? nullptr : create(TextureInfo(info.res, info.depthFormat, TextureUsage::Render_depth));

	std::vector<Texture*> textures(info.buffering * info.targets);

	for (u32 i = 0; i < (u32)textures.size(); ++i)
		textures[i] = create(TextureInfo(info.res, info.formats[i / buffering], TextureUsage::Render_target));

	RenderTarget *target = new RenderTarget(RenderTargetInfo(info.res, info.depthFormat, info.formats, info.buffering), depth, textures);

	if (!target->init(this))
		Log::throwError<Graphics, 0xD>("Couldn't initialize RenderTarget");

	return target;
}

Pipeline *Graphics::create(PipelineInfo info) {

	Pipeline *p = new Pipeline(info);
	if (!p->init(this))
		return (Pipeline*) Log::throwError<Graphics, 0x1D>("Couldn't initialize pipeline");

	return p;
}

PipelineState *Graphics::create(PipelineStateInfo info) {

	PipelineState *ps = new PipelineState(info);
	if (!ps->init(this))
		return (PipelineState*) Log::throwError<Graphics, 0x1E>("Couldn't initialize pipeline state");

	return ps;
}