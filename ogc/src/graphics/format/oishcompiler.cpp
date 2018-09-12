#include "graphics/format/oish.h"
#include "graphics/shader.h"
#include "api/spv/spvhelper.h"
using namespace oi::gc;
using namespace oi;

SHFile oiSH::convert(ShaderSource source) {

	ShaderInfo info = compile(source);

	if (info.path == "") {
		Log::error("Couldn't compile to oiSH file");
		return {};
	}

	return oiSH::convert(info);

}

ShaderInfo oiSH::compile(ShaderSource source) {

	if (source.getType() != ShaderSourceType::SPV)
		;	//TODO: Convert to SPV

	ShaderInfo info;
	info.path = source.getName();

	for (auto &elem : source.getSpv()) {

		ShaderStageType type = SpvHelper::pickType(elem.first);

		if (!SpvHelper::addStage(elem.second, type, info)) {
			Log::error("Couldn't add stage to shader");
			return {};
		}

	}

	return info;

}