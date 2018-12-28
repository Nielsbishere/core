#include "graphics/graphics.h"
#include "graphics/objects/shader/shaderdata.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/texture/sampler.h"
using namespace oi::gc;
using namespace oi;

ShaderData::~ShaderData() {

	for (auto &reg : info.shaderData)
		g->destroyObject(reg.second);

	destroyData();
}

bool ShaderData::init() {

	info.shaderData.reserve(info.registers.size());

	for (ShaderRegister &reg : info.registers)
		info.shaderData[reg.name] = nullptr;

	for (auto &sb : info.buffer)
		set(sb.first, g->create(getName() + " " + sb.first, info.buffer[sb.first]));

	changed = Bitset(g->getBuffering(), true);
	return initData();
}

bool ShaderData::set(String path, GraphicsResource *res) {

	auto it = info.shaderData.find(path);

	if (it == info.shaderData.end())
		return Log::warn(String("ShaderData::set(") + path + ") failed; the path couldn't be found");

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
			return Log::throwError<Shader, 0x0>(String("Shader::set(") + path + ") failed; invalid type (type is ShaderBuffer, but type provided isn't)");
		else if (type == ShaderRegisterType::Texture2D && !res->isType<Texture>() && !res->isType<VersionedTexture>() && !res->isType<TextureList>())
			return Log::throwError<Shader, 0x1>(String("Shader::set(") + path + ") failed; invalid type (type is Texture, TextureList or VersionedTexture, but type provided isn't)");
		else if (type == ShaderRegisterType::Sampler && !res->isType<Sampler>())
			return Log::throwError<Shader, 0x2>(String("Shader::set(") + path + ") failed; invalid type (type is Sampler, but type provided isn't)");
		else if (type == ShaderRegisterType::Image && (!res->isType<VersionedTexture>() || ((VersionedTexture*)res)->getFormat() != sreg.format || ((VersionedTexture*)res)->getUsage() != TextureUsage::Compute_target))
			return Log::throwError<Shader, 0x3>(String("Shader::set(") + path + ") failed; invalid type (type is Image, but type provided isn't)");

		if (res->isType<TextureList>() && ((TextureList*)res)->size() != sreg.size)
			return Log::throwError<Shader, 0x4>(String("Shader::set(") + path + ") failed; TextureList size incompatible with shader");

	}

	if (it->second != res) {

		changed.clear(true);

		if (it->second != nullptr)
			g->destroyObject(it->second);

		it->second = res;

		if (res != nullptr)
			g->use(res);
	}

	return true;
}