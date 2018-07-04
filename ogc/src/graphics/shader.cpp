#include "graphics/shader.h"
#include "graphics/sampler.h"
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

		if (isBuffer && dynamic_cast<ShaderBuffer*>(res) == nullptr)
			return Log::throwError<Shader, 0x1>(String("Shader::set(") + path + ") failed; invalid type (type is ShaderBuffer, but type mentioned isn't)");
		else if((type == ShaderRegisterType::Texture2D || type == ShaderRegisterType::Image) && dynamic_cast<Texture*>(res) == nullptr)
			return Log::throwError<Shader, 0x2>(String("Shader::set(") + path + ") failed; invalid type (type is Texture, but type mentioned isn't)");
		else if(type == ShaderRegisterType::Sampler && dynamic_cast<Sampler*>(res) == nullptr)
			return Log::throwError<Shader, 0x3>(String("Shader::set(") + path + ") failed; invalid type (type is Sampler, but type mentioned isn't)");

	}

	if (it->second != res)
		changed = true;

	it->second = res;
	return true;
}