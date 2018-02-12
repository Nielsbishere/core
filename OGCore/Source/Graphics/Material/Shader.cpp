#include "Graphics/Material/Shader.h"
#include "Graphics/GPU/BufferGPU.h"
using namespace oi::gc;
using namespace oi;

BufferVar Shader::get(OString path) {

	std::vector<OString> dirs = path.split(".");

	if (dirs.size() < 1) {
		Log::error(OString("Couldn't find SSBO with given path \"") + path + "\"");
		return {};
	}

	auto it = ssbos.find(dirs[0]);

	if (it == ssbos.end()) {
		Log::error(OString("Couldn't find SSBO with given path \"") + path + "\"");
		return {};
	}

	if (dirs.size() < 2)
		return it->second.structured[""];

	return it->second.structured[path.cutBegin(dirs[0].size() + 1)];
}

std::vector<OString> Shader::getBufferNames() {

	std::vector<OString> keys(ssbos.size());
	u32 i = 0;

	for (auto at : ssbos) {
		keys[i] = at.first;
		++i;
	}
	
	return keys;
}

std::vector<ShaderStorageBuffer> Shader::getBuffers() {

	std::vector<ShaderStorageBuffer> values(ssbos.size());
	u32 i = 0;

	for (auto at : ssbos) {
		values[i] = at.second;
		++i;
	}

	return values;
}