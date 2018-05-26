#include "graphics/shader.h"
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

	if (it->second != res)
		changed = true;

	//TODO: Check if type is correct

	it->second = res;
	return true;
}