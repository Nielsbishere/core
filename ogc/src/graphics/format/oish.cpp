#include "graphics/format/oish.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
#include <format/oisl.h>
#include <file/filemanager.h>
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

SHFile oiSH::convert(ShaderInfo info) {
	
	SHFile output;

	if (info.stages.size() == 0)
		Log::throwError<oiSH, 0x2>("oiSH::convert couldn't be executed; not enough stages");

	//Setup stringlist and vars

	output.stringlist.keyset = String::getDefaultCharset();
	output.stringlist.names.push_back(info.path);

	SHStageTypeFlag shaderFlag = SHStageTypeFlag::COMPUTE;

	u16 byteIndex = 0;

	//Convert stages

	output.stage.resize((u32) info.stages.size());

	if (info.stages.size() == 1) { //Compute

		ShaderStageInfo &istage = info.stages[0];

		if (istage.type != ShaderStageType::Compute_shader)
			Log::throwError<oiSH, 0x3>("oiSH::convert couldn't be executed; only one stage; and not Compute.");

		if (istage.code.size() == 0)
			Log::throwError<oiSH, 0x4>("oiSH::convert couldn't be executed; no bytecode");


		SHStage &ostage = output.stage[0];

		ostage.codeIndex = (u16) 0U;
		ostage.codeLength = (u16) istage.code.size();
		ostage.type = (u8) ShaderStageType::Compute_shader;
		ostage.flags = (u8) 0U;
		ostage.nameIndex = 0U;

		byteIndex += ostage.codeLength;

	} else {

		u32 i = 0;

		for (ShaderStageInfo &istage : info.stages) {

			if(istage.type == ShaderStageType::Compute_shader)
				Log::throwError<oiSH, 0x4>("oiSH::convert couldn't be executed; Graphics shaders can't contain Compute module.");

			if(istage.code.size() == 0)
				Log::throwError<oiSH, 0x5>("oiSH::convert couldn't be executed; no bytecode");

			SHStage &ostage = output.stage[i];

			ostage.codeIndex = byteIndex;
			ostage.codeLength = (u16)istage.code.size();
			ostage.type = (u8) istage.type.getValue();
			ostage.flags = (u8)0U;
			ostage.nameIndex = 0U;

			output.bytecode.resize(byteIndex + ostage.codeLength);
			memcpy(output.bytecode.data() + byteIndex, istage.code.addr(), istage.code.size());

			shaderFlag = (SHStageTypeFlag)((u32) shaderFlag | 1U << (istage.type.getValue() - 1U));

			byteIndex += ostage.codeLength;
			++i;
		}

	}

	//Registers

	output.registers.resize(info.registers.size());

	for (u32 i = 0; i < (u32)info.registers.size(); ++i) {

		ShaderRegister &reg = info.registers[i];

		if (reg.type.getValue() == 0U)
			Log::throwError<oiSH, 0x6>("Invalid register type");

		u32 representation = 0U;

		if (reg.type.getValue() <= ShaderRegisterType::SSBO) {

			representation = 1U;

			for (auto &sbi : info.buffer)
				if (sbi.first == reg.name)
					break;
				else
					++representation;

			if (representation > (u32)info.buffer.size())
				Log::throwError<oiSH, 0x6>("ShaderBuffer had no representation; this is illegal");

		}

		output.registers[i] = {

			(u8)reg.type.getValue(),
			(u8)reg.access.getValue(),
			(u16)representation,

			(u16)output.stringlist.names.size()
		};

		output.stringlist.names.push_back(reg.name);

	}

	//Convert outputs

	output.outputs.resize(info.output.size());

	for (u32 i = 0; i < (u32)info.output.size(); ++i) {

		ShaderOutput &out = info.output[i];

		output.outputs[i] = {
			(u8)out.type.getValue(),
			(u8)out.id,
			(u16)output.stringlist.names.size()
		};

		output.stringlist.names.push_back(out.name);

	}

	//Convert vb vars

	output.ivar.resize(info.var.size());

	for (u32 i = 0; i < (u32)info.var.size(); ++i) {

		ShaderVBVar &var = info.var[i];

		output.ivar[i] = {
			(u8) var.type.getValue(),
			(u16) output.stringlist.names.size()
		};

		output.stringlist.names.push_back(var.name);

	}

	//Convert buffers

	output.buffers.resize(info.buffer.size());

	for (auto &elem : info.buffer) {

		for (u32 i = 0; i < elem.second.elements.size(); ++i)
			output.stringlist.names.push_back(elem.second.elements[i].name);

		u32 id = 0U;

		for (auto &elem2 : info.bufferIds)
			if (elem2.second == elem.first) {
				id = elem2.first;
				break;
			}

		output.buffers[id] = oiSB::convert(elem.second, &output.stringlist);
	}

	//Setup header

	output.header = {

		{ 'o', 'i', 'S', 'H' },

		(u8)SHHeaderVersion::v0_0_1,
		(u8)shaderFlag,
		(u8)info.stages.size(),
		(u8) 0,

		(u8)info.var.size(),
		(u8)info.buffer.size(),
		(u8)info.output.size(),
		(u8)info.registers.size(),

		byteIndex,
		(u16) 0U
	};


	return output;
}

ShaderInfo oiSH::convert(Graphics *g, SHFile file) {

	ShaderInfo info;

	info.path = file.stringlist.names[0];

	std::vector<ShaderStage*> &stage = info.stage = std::vector<ShaderStage*>(file.stage.size());

	info.stages.resize((u32)stage.size());

	Buffer codeBuffer = Buffer::construct(file.bytecode.data(), (u32)file.bytecode.size());

	for (u32 i = 0; i < (u32)stage.size(); ++i) {

		Buffer b = codeBuffer.offset(file.stage[i].codeIndex);
		b = Buffer(b.addr(), file.stage[i].codeLength);

		stage[i] = g->create(info.path + " " + ShaderStageType(file.stage[i].type).getName(), info.stages[i] = ShaderStageInfo(b, ShaderStageType(file.stage[i].type)));
	}

	//Vertex inputs
	
	std::vector<ShaderVBVar> &var = info.var = std::vector<ShaderVBVar>(file.ivar.size());

	for (u32 i = 0; i < (u32)var.size(); ++i) {

		SHInputVar &v = file.ivar[i];
		TextureFormat format = TextureFormat(v.type);

		var[i] = ShaderVBVar(format, file.stringlist.names[v.nameIndex]);

	}

	//Registers

	auto &registers = info.registers = std::vector<ShaderRegister>(file.registers.size());

	for (u32 i = 0; i < (u32)file.registers.size(); ++i) {
		SHRegister &r = file.registers[i];

		ShaderRegister &reg = registers[i] = ShaderRegister(r.type, r.access, file.stringlist.names[r.nameIndex]);

		if (reg.type.getValue() == 0 || reg.access.getValue() == 0)
			Log::throwError<oiSH, 0x0>(String("ShaderRegister ") + reg.name + " is invalid");
	}

	//Outputs

	auto &outputs = info.output = std::vector<ShaderOutput>(file.outputs.size());

	for (u32 i = 0; i < (u32)file.outputs.size(); ++i) {
		SHOutput &o = file.outputs[i];

		ShaderOutput &out = outputs[i] = ShaderOutput(o.type, file.stringlist.names[o.nameIndex], o.id);

		if (out.type.getValue() == 0)
			Log::throwError<oiSH, 0x1>("Invalid shader output");
	}

	//Buffers

	auto &buffers = info.buffer;
	buffers.reserve(file.buffers.size());

	u32 bufId = 0U;

	for (u32 i = 0; i < (u32)file.registers.size(); ++i) {

		SHRegister &r = file.registers[i];

		if (r.type <= ShaderRegisterType::SSBO && r.type != ShaderRegisterType::Undefined) {

			u32 buf = r.representation;

			if (buf == 0U) 
				Log::throwError<oiSH, 0x1>("ShaderRegister of type Buffer (SSBO or UBO) doesn't reference a buffer");

			--buf;

			String name = file.stringlist.names[r.nameIndex];

			buffers[name] = oiSB::convert(g, file.buffers[buf], &file.stringlist);
			info.bufferIds[bufId] = name;
			++bufId;
		}
	}

	return info;
}

bool oiSH::read(String path, SHFile &file) {

	Buffer buf;

	if(!FileManager::get()->read(path, buf))
		return Log::error("Couldn't open file");

	if (!read(buf, file))
		return Log::error("Couldn't read file");

	buf.deconstruct();
	return true;
}

bool oiSH::read(Buffer buf, SHFile &file) {

	Buffer start = buf;

	if (buf.size() < sizeof(SHHeader))
		return Log::error("Invalid oiSH file");

	SHHeader &header = file.header = buf.operator[]<SHHeader>(0);
	buf = buf.offset((u32) sizeof(SHHeader));

	if (String(header.header, 4) != "oiSH")
		return Log::error("Invalid oiSH (header) file");
	
	SHHeaderVersion v(header.version);

	switch (v.getValue()) {

	case SHHeaderVersion::v0_0_1.value:
		goto v0_0_1;

	default:
		return Log::error("Invalid oiSH (header) file");

	}

	v0_0_1:
	{

		u32 stages = (u32)(header.shaders * sizeof(SHStage));
		u32 ivars = (u32)(header.inputAttributes * sizeof(SHInputVar));
		u32 registers = (u32)(header.registers * sizeof(SHRegister));
		u32 outputs = (u32)(header.outputs * sizeof(SHOutput));

		if (buf.size() < stages + ivars + registers + outputs)
			return Log::error("Invalid oiSH file; too small");

		file.stage.assign((SHStage*)buf.addr(), (SHStage*)(buf.addr() + stages));
		buf = buf.offset(stages);

		file.ivar.assign((SHInputVar*)buf.addr(), (SHInputVar*)(buf.addr() + ivars));
		buf = buf.offset(ivars);

		file.registers.assign((SHRegister*)buf.addr(), (SHRegister*)(buf.addr() + registers));
		buf = buf.offset(registers);

		file.outputs.assign((SHOutput*)buf.addr(), (SHOutput*)(buf.addr() + outputs));
		buf = buf.offset(outputs);

		SLFile &sl = file.stringlist;

		if (!oiSL::read(buf, sl))
			return Log::error("Invalid oiSH (oiSL) file");

		buf = buf.offset(sl.size);

		file.buffers.resize(header.buffers);

		for (u32 i = 0; i < header.buffers; ++i)
			if (!oiSB::read(buf, file.buffers[i]))
				return Log::error("Invalid oiSH (oiSB) file");
			else
				buf = buf.offset(file.buffers[i].size);

		file.bytecode.assign(buf.addr(), buf.addr() + header.codeSize);
		buf = buf.offset(header.codeSize);

		goto end;

	}

	end:

	file.size = (u32)(buf.addr() - start.addr());
	if(buf.size() == 0) file.size = start.size();

	Log::println(String("Successfully loaded oiSH file with version ") + v.getName() + " (" + file.size + " bytes)");

	return true;
}

Buffer oiSH::write(SHFile &file) {

	SHHeader &header = file.header;

	u32 stages = (u32)(header.shaders * sizeof(SHStage));
	u32 ivars = (u32)(header.inputAttributes * sizeof(SHInputVar));
	u32 registers = (u32)(header.registers * sizeof(SHRegister));
	u32 outputs = (u32)(header.outputs * sizeof(SHOutput));

	Buffer b = oiSL::write(file.stringlist);
	std::vector<Buffer> buffers(file.buffers.size());

	u32 bufferSize = 0U;

	for (u32 i = 0; i < (u32)buffers.size(); ++i) {
		buffers[i] = oiSB::write(file.buffers[i]);
		bufferSize += file.buffers[i].size;
	}

	file.size = (u32) sizeof(header) + stages + ivars + registers + outputs + header.codeSize + file.stringlist.size + bufferSize;

	Buffer output(file.size);
	Buffer write = output;

	memcpy(write.addr(), &header, sizeof(header));
	write = write.offset((u32) sizeof(header));

	memcpy(write.addr(), file.stage.data(), stages);
	write = write.offset(stages);

	memcpy(write.addr(), file.ivar.data(), ivars);
	write = write.offset(ivars);

	memcpy(write.addr(), file.registers.data(), registers);
	write = write.offset(registers);

	memcpy(write.addr(), file.outputs.data(), outputs);
	write = write.offset(outputs);

	write.copy(b, b.size(), 0, 0);
	write = write.offset(b.size());
	b.deconstruct();

	for (Buffer b : buffers) {
		write.copy(b, b.size(), 0, 0);
		write = write.offset(b.size());
		b.deconstruct();
	}

	write.copy(Buffer::construct(file.bytecode.data(), (u32)file.bytecode.size()), (u32)file.bytecode.size(), 0, 0);
	write = write.offset((u32)file.bytecode.size());

	return output;
}

bool oiSH::write(String path, SHFile &file) {

	Buffer buf = write(file);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}