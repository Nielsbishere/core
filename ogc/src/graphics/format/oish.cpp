#include "graphics/format/oish.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
#include <format/oisl.h>
#include <file/filemanager.h>
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//SHFile oiSH::convert(ShaderInfo info) {
//
//}

ShaderInfo oiSH::convert(Graphics *g, SHFile file) {

	ShaderInfo info;

	std::vector<ShaderStage*> &stage = info.stage = std::vector<ShaderStage*>(file.stage.size());

	Buffer codeBuffer = Buffer::construct(file.bytecode.data(), (u32)file.bytecode.size());

	for (u32 i = 0; i < (u32)stage.size(); ++i) {

		Buffer b = codeBuffer.offset(file.stage[i].codeIndex);
		b = Buffer(b.addr(), file.stage[i].codeLength);

		stage[i] = g->create(ShaderStageInfo(b, ShaderStageType(file.stage[i].type)));
	}

	//Vertex inputs

	std::vector<ShaderVBSection> &buf = info.section = std::vector<ShaderVBSection>(file.ibuffer.size());
	std::vector<ShaderVBVar> &var = info.var = std::vector<ShaderVBVar>(file.ivar.size());
	std::vector<u32> offsets(buf.size());

	for (u32 i = 0; i < (u32)buf.size(); ++i) {
		SHInputBuffer &b = file.ibuffer[i];
		buf[i] = ShaderVBSection(b.size, (bool)b.type);
	}

	for (u32 i = 0; i < (u32)var.size(); ++i) {

		SHInputVar &v = file.ivar[i];
		TextureFormat format = TextureFormat(v.type);

		var[i] = ShaderVBVar(v.buffer, offsets[v.buffer], format, file.stringlist.names[v.nameIndex]);
		offsets[v.buffer] += g->getFormatSize(format);

	}

	//Registers

	auto &registers = info.registers = std::vector<ShaderRegister>(file.registers.size());

	for (u32 i = 0; i < (u32)file.registers.size(); ++i) {
		SHRegister &r = file.registers[i];

		ShaderRegister &reg = registers[i] = ShaderRegister(r.type, r.access, file.stringlist.names[r.nameIndex]);

		if (reg.type.getValue() == 0 || reg.access.getValue() == 0)
			Log::throwError<oiSH, 0x0>(String("ShaderRegister ") + reg.name + " is invalid");
	}

	//Buffers

	auto &buffers = info.buffer;
	buffers.reserve(file.buffers.size());

	for (u32 i = 0; i < (u32)file.registers.size(); ++i) {

		SHRegister &r = file.registers[i];

		if (r.type < ShaderRegisterType::SSBO_write && r.type != ShaderRegisterType::Undefined) {

			u32 buf = r.representation;

			if (buf == 0U) 
				Log::throwError<oiSH, 0x1>("ShaderRegister of type Buffer (SSBO or UBO) doesn't reference a buffer");

			--buf;

			buffers[file.stringlist.names[r.nameIndex]] = oiSB::convert(g, file.buffers[buf], &file.stringlist);

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
		u32 ibuffers = (u32)(header.inputBuffers * sizeof(SHInputBuffer));
		u32 ivars = (u32)(header.inputAttributes * sizeof(SHInputVar));
		u32 iregisters = (u32)(header.registers * sizeof(SHRegister));
		u32 ioutputs = (u32)(header.outputs * sizeof(SHOutput));

		if (buf.size() < stages + ibuffers + ivars + iregisters + ioutputs)
			return Log::error("Invalid oiSH file; too small");

		file.stage.assign((SHStage*)buf.addr(), (SHStage*)(buf.addr() + stages));
		buf = buf.offset(stages);

		file.ibuffer.assign((SHInputBuffer*)buf.addr(), (SHInputBuffer*)(buf.addr() + ibuffers));
		buf = buf.offset(ibuffers);

		file.ivar.assign((SHInputVar*)buf.addr(), (SHInputVar*)(buf.addr() + ivars));
		buf = buf.offset(ivars);

		file.registers.assign((SHRegister*)buf.addr(), (SHRegister*)(buf.addr() + iregisters));
		buf = buf.offset(iregisters);

		file.outputs.assign((SHOutput*)buf.addr(), (SHOutput*)(buf.addr() + ioutputs));
		buf = buf.offset(ioutputs);

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

	Log::println(String("Successfully loaded oiSH file with version ") + v.getName());

	return true;
}