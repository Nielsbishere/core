#include <api/spv/spvhelper.h>
#include <utils/log.h>
#include <graphics/format/oish.h>
#include <graphics/shader.h>
#include <graphics/shaderstage.h>
#include <graphics/graphics.h>

#include <fstream>

using namespace oi;
using namespace oi::wc;
using namespace oi::gc;
using namespace spirv_cross;

int main(int argc, char *argv[]) {

	String path, shaderName;
	std::vector<String> extensions;

	if (argc < 4) return (int) Log::error("Incorrect usage: oish_gen.exe <pathToShader> <shaderName> [shaderStage extensions]");

	path = argv[1];
	shaderName = argv[2];
	
	for (int i = 3; i < argc; ++i)
		extensions.push_back(argv[i]);

	ShaderInfo info;
	info.path = shaderName;

	std::vector<ShaderStageInfo> &stageInfo = info.stages;
	stageInfo.resize(extensions.size());

	std::vector<String> names = { shaderName };

	u32 j = 0, k = 0;

	//Open the extensions' spirv and parse their data
	for (String &s : extensions) {

		ShaderStageType type = SpvHelper::pickType(s);

		//Load debug spirv (with all variable names)

		std::ifstream str((path + s + ".spv").toCString(), std::ios::binary);

		if (!str.good()) return (int)Log::error("Couldn't open that file");

		u32 length = (u32)str.rdbuf()->pubseekoff(0, std::ios_base::end);

		Buffer b(length);
		str.seekg(0, std::ios::beg);
		str.read((char*)b.addr(), b.size());

		str.close();

		if (b.size() % 4 != 0)
			Log::throwError<VkNull, 0x0>("SPIRV bytecode incorrect");

		std::vector<uint32_t> bytecode((u32*)b.addr(), (u32*)(b.addr() + b.size()));
		Compiler comp(move(bytecode));

		ShaderResources res = comp.get_shader_resources();
		ShaderRegisterAccess stageAccess = type.getName().replace("_shader", "");

		//Get the inputs
		if (type == ShaderStageType::Vertex_shader)
			SpvHelper::getStageInputs(comp, res, info.var);

		//Get the outputs
		if (type == ShaderStageType::Fragment_shader)
			SpvHelper::getStageOutputs(comp, res, info.output);

		//Get the registers

		if (!SpvHelper::addBuffers(comp, res, info, stageAccess))
			Log::throwError<VkNull, 0x1>("Couldn't add buffer");

		for (Resource &r : res.separate_images) {

			u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);
			bool isWriteable = comp.get_decoration(r.id, spv::DecorationNonWritable) == 0U;

			const std::vector<u32> &arr = comp.get_type(r.type_id).array;

			if (info.registers.size() <= binding)
				info.registers.resize(binding + 1U);

			ShaderRegister &reg = info.registers[binding];
			u32 size = arr.size() == 0 ? 1 : arr[0];

			if(reg.name == "")
				reg = ShaderRegister(isWriteable ? ShaderRegisterType::Image : ShaderRegisterType::Texture2D, stageAccess, r.name, size);
			else {

				reg.access = reg.access.getValue() | stageAccess.getValue();

				if (reg.access == ShaderRegisterAccess::Undefined)
					return (int)Log::error("Invalid register access");
			}

		}

		for (Resource &r : res.separate_samplers) {

			u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);

			if (info.registers.size() <= binding)
				info.registers.resize(binding + 1U);

			ShaderRegister &reg = info.registers[binding];

			if (reg.name == "")
				reg = ShaderRegister(ShaderRegisterType::Sampler, stageAccess, r.name, 1);
			else {

				reg.access = reg.access.getValue() | stageAccess.getValue();

				if (reg.access == ShaderRegisterAccess::Undefined)
					return (int)Log::error("Invalid register access");
			}

		}

		b.deconstruct();

		//Load optimized spirv

		std::ifstream ospv((path + s + ".ospv").toCString(), std::ios::binary);

		if (!ospv.good()) return (int) Log::error("Couldn't open that file");

		length = (u32) ospv.rdbuf()->pubseekoff(0, std::ios_base::end);

		b = Buffer(length);
		ospv.seekg(0, std::ios::beg);
		ospv.read((char*)b.addr(), b.size());
		stageInfo[j] = { b, type };
		ospv.close();

		++j;
	}

	SHFile shFile = oiSH::convert(info);
	Buffer b = oiSH::write(shFile);

	std::ofstream oish((path + ".oiSH").toCString(), std::ios::binary);

	if (!oish.good()) return (int)Log::error("Couldn't open that file");
	
	oish.write((char*)b.addr(), b.size());
	oish.close();

	b.deconstruct();

	Log::println(String("Successfully converted to ") + path + ".oiSH");

	return 1U;
}