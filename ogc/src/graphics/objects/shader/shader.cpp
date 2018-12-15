#include "graphics/objects/shader/shader.h"
#include "graphics/graphics.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/format/oish.h"
using namespace oi::gc;
using namespace oi;

Shader::Shader(ShaderInfo info) : info(info) { }
ShaderExt &Shader::getExtension() { return ext; }
const ShaderInfo &Shader::getInfo() { return info; }
bool Shader::isCompute() { return info.stage.size() == 1; }

bool Shader::set(String path, GraphicsResource *res) {

	auto it = info.shaderRegister.find(path);

	if (it == info.shaderRegister.end())
		return Log::throwError<Shader, 0x0>(String("Shader::set(") + path + ") failed; the path couldn't be found");

	if (res != nullptr) {

		ShaderRegisterType type;

		ShaderRegister sreg;

		for (ShaderRegister &reg : info.registers)
			if (reg.name == path) {
				type = reg.type;
				sreg = reg;
				break;
			}

		bool isBuffer = (type == ShaderRegisterType::SSBO || type == ShaderRegisterType::UBO);

		if (isBuffer && !res->isType<ShaderBuffer>())
			return Log::throwError<Shader, 0x1>(String("Shader::set(") + path + ") failed; invalid type (type is ShaderBuffer, but type provided isn't)");
		else if(type == ShaderRegisterType::Texture2D && !res->isType<Texture>() && !res->isType<VersionedTexture>() && !res->isType<TextureList>())
			return Log::throwError<Shader, 0x2>(String("Shader::set(") + path + ") failed; invalid type (type is Texture, TextureList or VersionedTexture, but type provided isn't)");
		else if(type == ShaderRegisterType::Sampler && !res->isType<Sampler>())
			return Log::throwError<Shader, 0x3>(String("Shader::set(") + path + ") failed; invalid type (type is Sampler, but type provided isn't)");
		else if(type == ShaderRegisterType::Image && (!res->isType<VersionedTexture>() || ((VersionedTexture*)res)->getUsage() != TextureUsage::Compute_target))
			return Log::throwError<Shader, 0x6>(String("Shader::set(") + path + ") failed; invalid type (type is Image, but type provided isn't)");

		if (res->isType<TextureList>() && ((TextureList*)res)->size() != sreg.size)
			return Log::throwError<Shader, 0x4>(String("Shader::set(") + path + ") failed; TextureList size incompatible with shader");

	}

	if (it->second != res) {
		changed = true;
		g->destroyObject(it->second);
	}

	it->second = res;
	g->use(it->second);
	return true;
}

bool Shader::init() {

	if (info.stages.size() == 0) {

		SHFile file;

		if (!oiSH::read(info.path, file))
			return (Shader*)Log::throwError<Shader, 0x5>("Couldn't read shader");

		String path = info.path;

		info = oiSH::convert(g, file);

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
				info.stage[i] = g->create(info.path + " " + info.stages[i].type.getName(), info.stages[i]);

		}

	}

	for (ShaderStage *ss : info.stage)
		g->use(ss);

	for (auto &gr : info.shaderRegister)
		g->use(gr.second);

	return initData();
}