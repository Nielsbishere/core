#include "file/filemanager.h"
#include "format/oisl.h"
#include "graphics/graphics.h"
#include "graphics/format/oish.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderstage.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

SHFile oiSH::convert(ShaderInfo info) {
	
	SHFile output;

	if (info.stages.size() == 0)
		Log::throwError<oiSH, 0x2>("oiSH::convert couldn't be executed; not enough stages");

	//Setup stringlist and vars

	output.stringlist.keyset = String::getDefaultCharset();
	output.stringlist.add(info.path);

	SHStageTypeFlag shaderFlag = SHStageTypeFlag::COMPUTE;

	u16 byteIndex = 0;

	//Convert stages

	output.stage.resize((u32) info.stages.size());
	std::vector<SHInput> inputs;
	std::vector<SHOutput> outputs;

	if (info.stages.size() == 1) {		//Compute

		ShaderStageInfo &istage = info.stages[0];

		if (istage.type != ShaderStageType::Compute_shader)
			Log::throwError<oiSH, 0x3>("oiSH::convert couldn't be executed; it only has one stage. Which is only allowed for compute");

		if (istage.code.size() == 0)
			Log::throwError<oiSH, 0x4>("oiSH::convert couldn't be executed; no bytecode");

		SHStage &ostage = output.stage[0];

		ostage.codeIndex = (u16) 0U;
		ostage.codeLength = (u16) istage.code.size();
		ostage.type = (u8) istage.type.getValue();
		ostage.flags = (u8) 0U;
		ostage.nameIndex = 0U;
		ostage.inputs = (u8)istage.input.size();
		ostage.outputs = (u8)istage.output.size();
		output.bytecode = istage.code;

		byteIndex += ostage.codeLength;

	} else {						//Graphics

		u32 codeSize = 0;
		for (ShaderStageInfo &ostage : info.stages)
			codeSize += ostage.code.size();

		u32 i = 0;
		output.bytecode = CopyBuffer(codeSize);

		for (ShaderStageInfo &istage : info.stages) {

			if(istage.type == ShaderStageType::Compute_shader)
				Log::throwError<oiSH, 0x5>("oiSH::convert couldn't be executed; Graphics shaders can't contain Compute module");

			if(istage.code.size() == 0)
				Log::throwError<oiSH, 0x6>("oiSH::convert couldn't be executed; no bytecode");

			SHStage &ostage = output.stage[i];

			ostage.codeIndex = byteIndex;
			ostage.codeLength = (u16)istage.code.size();
			ostage.type = (u8) istage.type.getValue();
			ostage.flags = (u8)0U;
			ostage.nameIndex = 0U;
			ostage.inputs = (u8) istage.input.size();
			ostage.outputs = (u8) istage.output.size();
			
			memcpy(output.bytecode.addr() + byteIndex, istage.code.addr(), istage.code.size());

			shaderFlag = (SHStageTypeFlag)((u32) shaderFlag | (1U << (istage.type.getValue() - 1)));

			inputs.resize(istage.input.size());
			outputs.resize(istage.output.size());

			for (u32 j = 0; j < (u32)istage.input.size(); ++j) {
				ShaderInput &var = istage.input[j];
				inputs[j] = {
					(u8)var.type.getValue(),
					(u16)output.stringlist.add(var.name)
				};
			}

			for (u32 j = 0; j < (u32)istage.output.size(); ++j) {
				ShaderOutput &out = istage.output[j];
				outputs[j] = {
					(u8)out.type.getValue(),
					(u8)out.id,
					(u16)output.stringlist.add(out.name)
				};
			}

			output.stageInputs[ostage.type] = inputs;
			output.stageOutputs[ostage.type] = outputs;

			byteIndex += ostage.codeLength;
			++i;
		}

	}

	//Registers

	output.registers.resize(info.registers.size());

	for (u32 i = 0; i < (u32)info.registers.size(); ++i) {

		ShaderRegister &reg = info.registers[i];

		if (reg.type.getValue() == 0U)
			Log::throwError<oiSH, 0x7>("Invalid register type");

		output.registers[i] = {

			(u8)reg.type.getValue(),
			(u8)reg.id,
			(u16)0,

			(u16)output.stringlist.add(reg.name),
			(u16)reg.size,

			(u16)reg.access,
			(u8)reg.format.getValue()

		};

	}

	//Convert buffers

	output.buffers.resize(info.buffer.size());

	for (auto &elem : info.buffer) {

		for (u32 i = 0; i < elem.second.elements.size(); ++i)
			output.stringlist.add(elem.second.elements[i].name);

		u32 id = 0U;

		for (auto &rep : output.registers)
			if (output.stringlist.names[rep.nameIndex] == elem.first) {
				rep.representation = (u16)(id + 1);
				break;
			} else if (rep.type <= u32(ShaderRegisterType::SSBO))
				++id;

		output.buffers[id] = oiSB::convert(elem.second, &output.stringlist);
	}

	//Setup header

	output.header = {

		{ 'o', 'i', 'S', 'H' },

		(u8)SHVersion::v0_1,
		(u16)shaderFlag,
		(u8)info.stages.size(),

		(u8)info.buffer.size(),
		(u8)info.registers.size(),
		byteIndex,

		(u16)info.computeThreads.x,
		(u16)info.computeThreads.y,

		(u16)info.computeThreads.z,
		(u16)0

	};

	return output;
}

ShaderInfo oiSH::convert(Graphics *g, SHFile file) {

	ShaderInfo info;

	info.path = file.stringlist.names[0];

	std::vector<ShaderStage*> &stage = info.stage = std::vector<ShaderStage*>(file.stage.size());

	info.stages.resize((u32)stage.size());
	info.computeThreads = Vec3u(*(TVec3<u16>*) &file.header.groupX);

	Buffer codeBuffer = Buffer::construct(file.bytecode.addr(), (u32)file.bytecode.size());

	std::vector<ShaderInput> inputs;
	std::vector<ShaderOutput> outputs;

	for (u32 i = 0; i < (u32)stage.size(); ++i) {

		SHStage &st = file.stage[i];

		Buffer b = codeBuffer.offset(st.codeIndex);
		b = Buffer::construct(b.addr(), st.codeLength);

		inputs.resize(st.inputs);
		outputs.resize(st.outputs);

		std::vector<SHInput> &var = file.stageInputs[st.type];
		std::vector<SHOutput> &output = file.stageOutputs[st.type];

		for (u32 j = 0; j < (u32)var.size(); ++j) {
			SHInput &v = var[j];
			TextureFormat format = TextureFormat(v.type);
			inputs[j] = ShaderInput(format, file.stringlist.names[v.nameIndex]);
		}

		for (u32 j = 0; j < (u32) output.size(); ++j) {

			SHOutput &o = output[j];
			ShaderOutput &out = outputs[j] = ShaderOutput(o.type, file.stringlist.names[o.nameIndex], o.id);

			if (out.type.getValue() == 0)
				Log::throwError<oiSH, 0x1>("Invalid shader output");
		}

		if (st.type == ShaderStageType::Vertex_shader)
			info.inputs = inputs;
		else if (st.type == ShaderStageType::Fragment_shader)
			info.outputs = outputs;

		stage[i] = g->create(info.path + " " + ShaderStageType(file.stage[i].type).getName(), info.stages[i] = ShaderStageInfo(b, ShaderStageType(file.stage[i].type), inputs, outputs));
	}

	//Registers

	auto &registers = info.registers = std::vector<ShaderRegister>(file.registers.size());

	for (u32 i = 0; i < (u32)file.registers.size(); ++i) {
		SHRegister &r = file.registers[i];

		ShaderRegister &reg = registers[i] = ShaderRegister(r.type, (ShaderAccessType) r.access, file.stringlist.names[r.nameIndex], (u32) r.size, r.id, TextureFormat::find(r.format));

		if (reg.type.getValue() == 0)
			Log::throwError<oiSH, 0x0>(String("ShaderRegister ") + reg.name + " is invalid");
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
				Log::throwError<oiSH, 0x8>("ShaderRegister of type Buffer (SSBO or UBO) doesn't reference a buffer");

			--buf;

			String name = file.stringlist.names[r.nameIndex];

			buffers[name] = oiSB::convert(file.buffers[buf], &file.stringlist);
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

	if (String(4, header.header) != "oiSH")
		return Log::error("Invalid oiSH (header) file");
	
	switch ((SHVersion)header.version) {

	case SHVersion::v0_1:
		goto v0_1;

	default:
		return Log::error("Invalid oiSH (header) file");

	}

	v0_1:
	{

		u32 stages = (u32)(header.shaders * sizeof(SHStage));
		u32 registers = (u32)(header.registers * sizeof(SHRegister));

		if (buf.size() < stages)
			return Log::error("Invalid oiSH file; invalid size");

		file.stage.assign((SHStage*)buf.addr(), (SHStage*)(buf.addr() + stages));
		buf = buf.offset(stages);

		for (u32 i = 0; i < header.shaders; ++i) {

			SHStage &stage = file.stage[i];

			u32 ivars = (u32)(stage.inputs * sizeof(SHInput));
			u32 outputs = (u32)(stage.outputs * sizeof(SHOutput));

			if(buf.size() < registers + ivars + outputs)
				return Log::error("Invalid oiSH file; invalid size");

			std::vector<SHInput> &input = file.stageInputs[stage.type];
			std::vector<SHOutput> &output = file.stageOutputs[stage.type];

			input.assign((SHInput*)buf.addr(), (SHInput*)(buf.addr() + ivars));
			buf = buf.offset(ivars);

			output.assign((SHOutput*)buf.addr(), (SHOutput*)(buf.addr() + outputs));
			buf = buf.offset(outputs);

		}

		file.registers.assign((SHRegister*)buf.addr(), (SHRegister*)(buf.addr() + registers));
		buf = buf.offset(registers);

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

		file.bytecode = CopyBuffer(buf.addr(), header.codeSize);
		buf = buf.offset(header.codeSize);

		goto end;

	}

	end:

	file.size = (u32)(buf.addr() - start.addr());
	if(buf.size() == 0) file.size = start.size();

	Log::println(String("Successfully loaded oiSH file with version v") + (file.header.version / 10) + "_" + (file.header.version % 10) + " (" + file.size + " bytes)");

	return true;
}

Buffer oiSH::write(SHFile &file) {

	SHHeader &header = file.header;

	u32 stages = (u32)(header.shaders * sizeof(SHStage));
	u32 stageInOut = 0;
	u32 registers = (u32)(header.registers * sizeof(SHRegister));

	for (u32 i = 0, j = header.shaders; i < j; ++i) {
		SHStage &stage = file.stage[i];
		stageInOut += stage.inputs * (u32)sizeof(SHInput) + stage.outputs * (u32)sizeof(SHOutput);
	}

	Buffer b = oiSL::write(file.stringlist);
	std::vector<Buffer> buffers(file.buffers.size());

	u32 bufferSize = 0U;

	for (u32 i = 0; i < (u32)buffers.size(); ++i) {
		buffers[i] = oiSB::write(file.buffers[i]);
		bufferSize += file.buffers[i].size;
	}

	file.size = (u32) sizeof(header) + stages + stageInOut + registers + header.codeSize + file.stringlist.size + bufferSize;

	Buffer output(file.size);
	Buffer write = output;

	memcpy(write.addr(), &header, sizeof(header));
	write = write.offset((u32) sizeof(header));

	memcpy(write.addr(), file.stage.data(), stages);
	write = write.offset(stages);

	for (u32 i = 0, j = header.shaders; i < j; ++i) {

		SHStage &stage = file.stage[i];

		u32 ivars = stage.inputs * (u32)sizeof(SHInput), outputs = stage.outputs * (u32)sizeof(SHOutput);

		memcpy(write.addr(), file.stageInputs[stage.type].data(), ivars);
		write = write.offset(ivars);

		memcpy(write.addr(), file.stageOutputs[stage.type].data(), outputs);
		write = write.offset(outputs);

	}

	memcpy(write.addr(), file.registers.data(), registers);
	write = write.offset(registers);

	write.copy(b, b.size(), 0, 0);
	write = write.offset(b.size());
	b.deconstruct();

	for (Buffer &buffer : buffers) {
		write.copy(buffer, buffer.size(), 0, 0);
		write = write.offset(buffer.size());
		buffer.deconstruct();
	}

	write.copy(Buffer::construct(file.bytecode.addr(), (u32)file.bytecode.size()), (u32)file.bytecode.size(), 0, 0);
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