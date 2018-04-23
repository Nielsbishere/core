#include "graphics/format/oish.h"
#include "graphics/shader.h"
#include "graphics/shaderstage.h"
#include "graphics/graphics.h"
#include <format/oisl.h>
using namespace oi::gc;
using namespace oi;

bool oiSH::read(Graphics *g, String path, ShaderInfo &info) {

	path = path + ".oiSH";

	//Get file

	SHFile file;

	if (!read(path, file))
		return Log::error("Couldn't read SH file");

	//Convert it into ShaderInfo data

	std::vector<ShaderStage*> &stage = info.stage = std::vector<ShaderStage*>(file.stage.size());

	Buffer codeBuffer = Buffer::construct(file.bytecode.data(), (u32) file.bytecode.size());

	for (u32 i = 0; i < (u32)stage.size(); ++i) {

		Buffer b = codeBuffer.offset(file.stage[i].codeIndex);
		b = Buffer(b.addr(), file.stage[i].codeLength);

		stage[i] = g->create(ShaderStageInfo(b, ShaderStageType(file.stage[i].type)));
	}

	std::vector<ShaderVBSection> &buf = info.section = std::vector<ShaderVBSection>(file.ibuffer.size());
	std::vector<ShaderVBVar> &var = info.var = std::vector<ShaderVBVar>(file.ivar.size());
	std::vector<u32> offsets(buf.size());

	for (u32 i = 0; i < (u32) buf.size(); ++i) {
		SHInputBuffer &b = file.ibuffer[i];
		buf[i] = ShaderVBSection(b.size, (bool) b.type);
	}

	for (u32 i = 0; i < (u32)var.size(); ++i) {

		SHInputVar &v = file.ivar[i];
		TextureFormat format = TextureFormat(v.type);

		var[i] = ShaderVBVar(v.buffer, offsets[v.buffer], format);
		offsets[v.buffer] += g->getFormatSize(format);

	}

	return true;
}

bool oiSH::read(String path, SHFile &file) {

	Buffer buf = Buffer::readFile(path);

	if (buf.size() == 0)
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

		if (buf.size() < stages + ibuffers + ivars)
			return Log::error("Invalid oiSH file; too small");

		file.stage.assign((SHStage*)buf.addr(), (SHStage*)(buf.addr() + stages));
		buf = buf.offset(stages);

		file.ibuffer.assign((SHInputBuffer*)buf.addr(), (SHInputBuffer*)(buf.addr() + ibuffers));
		buf = buf.offset(ibuffers);

		file.ivar.assign((SHInputVar*)buf.addr(), (SHInputVar*)(buf.addr() + ivars));
		buf = buf.offset(ivars);

		SLFile &sl = file.stringlist;

		if (!oiSL::read(buf, sl))
			return Log::error("Invalid oiSH (oiSL) file");

		buf = buf.offset(sl.size);

		file.bytecode.assign(buf.addr(), buf.addr() + header.codeSize);
		buf = buf.offset(header.codeSize);

		goto end;

	}

	end:

	file.size = (u32)(buf.addr() - start.addr());

	Log::println(String("Successfully loaded oiSH file with version ") + v.getName());

	return true;
}