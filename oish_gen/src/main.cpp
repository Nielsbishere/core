#include "spirv_cross.h"
#include <utils/log.h>
#include <graphics/format/oish.h>
#include <graphics/shader.h>
#include <graphics/shaderstage.h>
#include <graphics/graphics.h>

#include <fstream>

using namespace oi;
using namespace oi ::wc;
using namespace oi::gc;
using namespace spirv_cross;

ShaderStageType pickExtension(String &s) {
	if (s == ".vert") return ShaderStageType::Vertex_shader;
	if (s == ".frag") return ShaderStageType::Fragment_shader;
	if (s == ".geom") return ShaderStageType::Geometry_shader;
	if (s == ".comp") return ShaderStageType::Compute_shader;
	Log::throwError<ShaderStageType, 0x0>("Couldn't pick a shader stage type from string; so extension is invalid");
	return ShaderStageType::Undefined;
}

TextureFormat getFormat(SPIRType type) {

	switch (type.basetype) {

	case SPIRType::BaseType::Half:
		return TextureFormat::R16f - (type.vecsize - 1);

	case SPIRType::BaseType::Float:
		return TextureFormat::R32f - (type.vecsize - 1);

	case SPIRType::BaseType::UInt:
		return TextureFormat::R32u - (type.vecsize - 1);

	case SPIRType::BaseType::Int:
		return TextureFormat::R32i - (type.vecsize - 1);

	case SPIRType::BaseType::UInt64:
		return TextureFormat::R64u - (type.vecsize - 1);

	case SPIRType::BaseType::Double:
		return TextureFormat::R64f - (type.vecsize - 1);

	case SPIRType::BaseType::Boolean:
	case SPIRType::BaseType::Char:
		return TextureFormat::R32u;

	default:
		return TextureFormat::Undefined;

	}


}

void fillStruct(Compiler &comp, u32 id, ShaderBufferInfo &info, ShaderBufferObject *var) {

	auto &type = comp.get_type(id);
	
	for (u32 i = 0; i < (u32)type.member_types.size(); ++i) {

		ShaderBufferObject obj;

		const SPIRType &mem = comp.get_type(type.member_types[i]);

		obj.offset = (u32) comp.type_struct_member_offset(type, i);
		obj.name = comp.get_member_name(type.parent_type == 0 ? id : type.parent_type, i);

		u32 varId = var == &info.self ? 0U : (u32)(var - info.elements.data()) + 1U;

		u32 flags = 0;

		if (mem.array.size() != 0) {
			flags |= (u32)SBVarFlag::IS_ARRAY;

			if (mem.array[mem.array.size() - 1] == 0) {
				flags |= (u32)SBVarFlag::IS_DYNAMIC;
				info.allocate = false;
			}
		}

		if (mem.basetype == SPIRType::Struct) {

			u32 size = (u32)comp.get_declared_struct_member_size(type, i);

			if (size == 0 && (flags & (u32)SBVarFlag::IS_DYNAMIC) != 0) {	//Go through each member and calculate size

				for (u32 j = 0; j < (u32)mem.member_types.size(); ++j) {

					const SPIRType &mmem = comp.get_type(mem.member_types[j]);

					u32 count = mmem.columns;

					for (u32 k : mmem.array)
						count *= k;

					if(mmem.basetype == SPIRType::Struct)
						size += count * (u32) comp.get_declared_struct_member_size(mem, j);
					else
						size += count * Graphics::getFormatSize(getFormat(mmem));

				}

			}

			obj.length = size;
			obj.arr = mem.array;
			obj.format = TextureFormat::Undefined;

			obj.flags = (SBOFlag)flags;

			u32 objoff = (u32) info.elements.size();

			info.push(obj, *var);
			var = varId == 0 ? &info.self : info.elements.data() + varId - 1U;

			fillStruct(comp, type.member_types[i], info, info.elements.data() + objoff);

		} else {

			obj.format = getFormat(mem);
			obj.arr = mem.array;
			obj.length = Graphics::getFormatSize(obj.format);
			
			if (mem.columns != 1) {
				obj.arr.insert(obj.arr.begin(), mem.columns);
				flags |= (u32)SBVarFlag::IS_MATRIX;
			}

			obj.flags = (SBOFlag)flags;

			info.push(obj, *var);
			var = varId == 0 ? &info.self : info.elements.data() + varId - 1U;
		}
	}

}

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

		ShaderStageType type = pickExtension(s);

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

		//Get the inputs
		if (type == ShaderStageType::Vertex_shader) {

			//The variables that we're going to be filling in
			std::vector<ShaderVBVar> &vars = info.var;
			vars.resize(res.stage_inputs.size());

			u32 i = 0;

			//Convert the inputs from Resource (res.stage_inputs) to ShaderVBVar and ShaderVBSection
			for (Resource &r : res.stage_inputs) {

				SPIRType type = comp.get_type_from_variable(r.id);
				vars[i].type = getFormat(type);
				u32 varSize = Graphics::getFormatSize(vars[i].type) * type.columns;

				vars[i].name = r.name;

				++i;
			}
		}

		//Get the outputs
		if (type == ShaderStageType::Fragment_shader) {

			info.output.resize(res.stage_outputs.size());

			u32 i = 0;
			for (Resource &r : res.stage_outputs) {
				info.output[i] = ShaderOutput(getFormat(comp.get_type_from_variable(r.id)), r.name, comp.get_decoration(r.id, spv::DecorationLocation));
				++i;
			}

		}

		//Get the registers

		std::vector<Resource> buf = res.uniform_buffers;
		buf.insert(buf.end(), res.storage_buffers.begin(), res.storage_buffers.end());

		ShaderRegisterAccess stageAccess = type.getName().replace("_shader", "");

		u32 i = 0;
		for (Resource &r : buf) {

			u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);
			
			bool isUBO = i < res.uniform_buffers.size();

			ShaderRegisterType stype = !isUBO ? 2U : 1U;

			if(info.registers.size() <= binding)
				info.registers.resize(binding + 1U);

			ShaderRegister &reg = info.registers[binding];

			if (reg.name == "") 
				reg = ShaderRegister(stype, stageAccess, r.name, 1);
			else {

				reg.access = reg.access.getValue() | stageAccess.getValue();

				if (reg.access == ShaderRegisterAccess::Undefined)
					return (int)Log::error("Invalid register access");
			}

			info.bufferIds[k] = r.name;
			ShaderBufferInfo &dat = info.buffer[r.name];

			const SPIRType &btype = comp.get_type(r.base_type_id);

			dat.size = (u32) comp.get_declared_struct_size(btype);
			dat.allocate = true;
			dat.type = reg.type;

			dat.self.length = dat.size;
			dat.self.format = TextureFormat::Undefined;
			dat.self.name = r.name;
			dat.self.offset = 0U;
			dat.self.parent = nullptr;

			fillStruct(comp, r.base_type_id, dat, &dat.self);

			++i;
			++k;
		}

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