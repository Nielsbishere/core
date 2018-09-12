#include "api/spv/spvhelper.h"
#include "graphics/format/oish.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;
using namespace spirv_cross;

ShaderStageType SpvHelper::pickType(const String &s) {
	if (s == ".vert") return ShaderStageType::Vertex_shader;
	if (s == ".frag") return ShaderStageType::Fragment_shader;
	if (s == ".geom") return ShaderStageType::Geometry_shader;
	if (s == ".comp") return ShaderStageType::Compute_shader;
	Log::throwError<ShaderStageType, 0x0>("Couldn't pick a shader stage type from string; so extension is invalid");
	return ShaderStageType::Undefined;
}

TextureFormat SpvHelper::getFormat(SPIRType type) {

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

void SpvHelper::fillStruct(Compiler &comp, u32 id, ShaderBufferInfo &info, ShaderBufferObject *var) {

	auto &type = comp.get_type(id);

	for (u32 i = 0; i < (u32)type.member_types.size(); ++i) {

		ShaderBufferObject obj;

		const SPIRType &mem = comp.get_type(type.member_types[i]);

		obj.offset = (u32)comp.type_struct_member_offset(type, i);
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

			if (size == 0)	//Fetch size if it can't be found (dynamically sized array)
				size = (u32)comp.get_declared_struct_size(comp.get_type(mem.self));

			obj.length = size;
			obj.arr = mem.array;
			obj.format = TextureFormat::Undefined;

			obj.flags = (SBOFlag)flags;

			u32 objoff = (u32)info.elements.size();

			info.push(obj, *var);
			var = varId == 0 ? &info.self : info.elements.data() + varId - 1U;

			fillStruct(comp, type.member_types[i], info, info.elements.data() + objoff);

		}
		else {

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

void SpvHelper::getStageOutputs(spirv_cross::Compiler &comp, spirv_cross::ShaderResources &res, std::vector<ShaderOutput> &output) {

	output.resize(res.stage_outputs.size());

	u32 i = 0;
	for (Resource &r : res.stage_outputs) {
		output[i] = ShaderOutput(SpvHelper::getFormat(comp.get_type_from_variable(r.id)), r.name, comp.get_decoration(r.id, spv::DecorationLocation));
		++i;
	}

}

void SpvHelper::getStageInputs(spirv_cross::Compiler &comp, spirv_cross::ShaderResources &res, std::vector<ShaderVBVar> &output) {

	output.resize(res.stage_inputs.size());

	u32 i = 0;
	for (Resource &r : res.stage_inputs) {
		output[i] = ShaderVBVar(SpvHelper::getFormat(comp.get_type_from_variable(r.id)), r.name);
		++i;
	}

}

void SpvHelper::getBuffer(spirv_cross::Compiler &comp, Resource &r, ShaderRegister &reg, ShaderBufferInfo &dat) {

	const SPIRType &btype = comp.get_type(r.base_type_id);

	dat.size = (u32)comp.get_declared_struct_size(btype);
	dat.allocate = true;
	dat.type = reg.type;

	dat.self.length = dat.size;
	dat.self.format = TextureFormat::Undefined;
	dat.self.name = r.name;
	dat.self.offset = 0U;
	dat.self.parent = nullptr;

	SpvHelper::fillStruct(comp, r.base_type_id, dat, &dat.self);

}

bool SpvHelper::addBuffers(spirv_cross::Compiler &comp, ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess stageAccess) {

	std::vector<Resource> buf = res.uniform_buffers;
	buf.insert(buf.end(), res.storage_buffers.begin(), res.storage_buffers.end());

	u32 i = 0;
	for (Resource &r : buf) {

		u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);

		bool isUBO = i < res.uniform_buffers.size();

		ShaderRegisterType stype = !isUBO ? ShaderRegisterType::SSBO : ShaderRegisterType::UBO;

		auto itt = std::find_if(info.registers.begin(), info.registers.end(), [binding](const ShaderRegister &reg) -> bool { return binding == reg.id; });

		if (itt == info.registers.end()) {
			info.registers.push_back(ShaderRegister(stype, stageAccess, r.name, 1, binding));
			itt = info.registers.end() - 1;
		} else {

			ShaderRegisterAccess access = itt->access.getValue() | stageAccess.getValue();

			if (access == ShaderRegisterAccess::Undefined)
				return Log::error("Invalid register access");

			itt->access = access;

		}
		
		auto it = info.buffer.find(r.name);

		if (it == info.buffer.end()) {
			ShaderBufferInfo &dat = info.buffer[r.name];
			SpvHelper::getBuffer(comp, r, *itt, dat);
		}

		++i;
	}

	return true;

}

bool SpvHelper::addTextures(Compiler &comp, ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess stageAccess) {

	for (Resource &r : res.separate_images) {

		u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);
		bool isWriteable = comp.get_decoration(r.id, spv::DecorationNonWritable) == 0U;

		const std::vector<u32> &arr = comp.get_type(r.type_id).array;
		u32 size = arr.size() == 0 ? 1 : arr[0];

		auto itt = std::find_if(info.registers.begin(), info.registers.end(), [binding](const ShaderRegister &reg) -> bool { return binding == reg.id; });

		if (itt == info.registers.end()) {
			info.registers.push_back(ShaderRegister(ShaderRegisterType::Texture2D, stageAccess, r.name, size, binding));
			itt = info.registers.end() - 1;
		} else {

			ShaderRegisterAccess access = itt->access.getValue() | stageAccess.getValue();

			if (access == ShaderRegisterAccess::Undefined)
				return Log::error("Invalid register access");

			itt->access = access;

		}

	}

	return true;

}

bool SpvHelper::addSamplers(Compiler &comp, ShaderResources &res, ShaderInfo &info, ShaderRegisterAccess stageAccess) {

	for (Resource &r : res.separate_samplers) {

		u32 binding = comp.get_decoration(r.id, spv::DecorationBinding);

		auto itt = std::find_if(info.registers.begin(), info.registers.end(), [binding](const ShaderRegister &reg) -> bool { return binding == reg.id; });

		if (itt == info.registers.end()) {
			info.registers.push_back(ShaderRegister(ShaderRegisterType::Sampler, stageAccess, r.name, 1, binding));
			itt = info.registers.end() - 1;
		} else {

			ShaderRegisterAccess access = itt->access.getValue() | stageAccess.getValue();

			if (access == ShaderRegisterAccess::Undefined)
				return Log::error("Invalid register access");

			itt->access = access;

		}

	}

	return true;

}

bool SpvHelper::addResources(spirv_cross::Compiler &comp, ShaderStageType type, ShaderInfo &info) {

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
		return Log::error("Couldn't add buffers");

	if (!SpvHelper::addTextures(comp, res, info, stageAccess))
		return Log::error("Couldn't add textures");

	if (!SpvHelper::addSamplers(comp, res, info, stageAccess))
		return Log::error("Couldn't add textures");

	return true;

}

bool SpvHelper::addStage(const CopyBuffer &b, ShaderStageType type, ShaderInfo &info) {

	if (b.size() % 4 != 0 || b.size() == 0)
		return Log::error("SPIR-V Bytecode invalid");

	std::vector<uint32_t> bytecode((u32*)b.addr(), (u32*)(b.addr() + b.size()));
	Compiler comp(move(bytecode));

	if (!SpvHelper::addResources(comp, type, info))
		return Log::error("Couldn't add stage resources to shader");

	#ifndef __DEBUG__
	//TODO: Optimize
	#endif

	info.stages.push_back(ShaderStageInfo(b, type));

	return true;
}