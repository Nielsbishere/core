#include "graphics/format/oisb.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "graphics/shaderbuffer.h"
#include <format/oisl.h>
#include <types/buffer.h>
#include <file/filemanager.h>
#include <cmath>
using namespace oi::wc;
using namespace oi::gc;
using namespace oi;

SBStruct::SBStruct(u16 nameIndex, u16 parent, u32 offset, u16 arraySize, u8 flags, u32 length) : nameIndex(nameIndex), parent(parent), offset(offset), arraySize(arraySize), length(length) {}
SBStruct::SBStruct() {}

SBVar::SBVar(u16 nameIndex, u16 parent, u32 offset, u32 arraySize, u8 type, u8 flags) : nameIndex(nameIndex), parent(parent), offset(offset), arraySize(arraySize), type(type), flags(flags) { }
SBVar::SBVar() {}

SBFile::SBFile(std::vector<SBStruct> structs, std::vector<SBVar> vars) : structs(structs), vars(vars) {}
SBFile::SBFile() {}


ShaderBufferInfo oiSB::convert(Graphics *g, SBFile file, SLFile *names) {

	u32 structc = (u32)file.structs.size();

	auto shaderBuffer = ShaderBufferInfo(ShaderRegisterType((file.header.flags & 0x3U) + 1U), file.header.bufferSize, structc + (u32)file.vars.size(), file.header.flags & 0x4U);

	for (u32 i = 0; i < (u32)shaderBuffer.elements.size(); ++i) {

		if (i < structc) {

			SBStruct sbst = file.structs[i];

			ShaderBufferObject *parent = sbst.parent == 0 ? &shaderBuffer.self : &shaderBuffer[sbst.parent - 1U];

			shaderBuffer[i] = {
				parent,
				sbst.offset,
				sbst.length,
				sbst.arraySize,
				names == nullptr ? sbst.nameIndex : names->names[sbst.nameIndex],
				TextureFormat::Undefined,
				sbst.flags
			};

			if (parent != nullptr)
				parent->addChild(&shaderBuffer[i]);
			else
				shaderBuffer.addRoot(&shaderBuffer[i]);

		}
		else {

			SBVar sbva = file.vars[i - structc];

			ShaderBufferObject *parent = sbva.parent == 0 ? &shaderBuffer.self : &shaderBuffer[sbva.parent - 1U];

			shaderBuffer[i] = {
				parent,
				sbva.offset,
				g->getFormatSize(TextureFormat(sbva.type)),
				sbva.arraySize,
				names == nullptr ? sbva.nameIndex : names->names[sbva.nameIndex],
				TextureFormat(sbva.type),
				sbva.flags
			};

			if (parent != nullptr)
				parent->addChild(&shaderBuffer[i]);
			else
				shaderBuffer.addRoot(&shaderBuffer[i]);
		}

	}

	return shaderBuffer;

}

bool oiSB::read(String path, SBFile &file) {

	Buffer buf;
	FileManager::get()->read(path, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't open file");

	if (!read(buf, file))
		return Log::error("Couldn't read file");

	buf.deconstruct();
	return true;
}

bool oiSB::read(Buffer buf, SBFile &file) {

	Buffer start = buf;

	if (buf.size() < sizeof(SBHeader))
		return Log::error("Invalid oiSB file");

	SBHeader &header = file.header = buf.operator[]<SBHeader>(0);
	buf = buf.offset((u32) sizeof(SBHeader));

	if (String(header.header, 4) != "oiSB")
		return Log::error("Invalid oiSB (header) file");

	SBHeaderVersion v(header.version);

	switch (v.getValue()) {

	case SBHeaderVersion::v0_1.value:
		goto v0_1;

	default:
		return Log::error("Invalid oiSB (header) file");

	}

v0_1:
	{

		u32 structsiz = header.structs * (u32) sizeof(SBStruct);
		u32 varsiz = header.vars * (u32) sizeof(SBVar);
		u32 siz = structsiz + varsiz;

		if (buf.size() < siz)
			return Log::error("Invalid oiSB file");

		file.structs.assign((SBStruct*) buf.addr(), (SBStruct*)(buf.addr() + structsiz));
		buf = buf.offset(structsiz);

		file.vars.assign((SBVar*)buf.addr(), (SBVar*)(buf.addr() + varsiz));
		buf = buf.offset(varsiz);

		goto end;

	}

end:

	if (buf.addr() != nullptr)
		file.size = (u32)(buf.addr() - start.addr());
	else
		file.size = start.size();

	Log::println(String("Successfully loaded oiSB file with version ") + v.getName() + " (" + file.size + " bytes)");

	return true;
}

SBFile oiSB::convert(ShaderBufferInfo &info, SLFile *names) {

	SBFile file;

	for (u32 i = 0; i < info.elements.size(); ++i) {

		auto &elem = info[i];

		if (elem.format != TextureFormat::Undefined)
			file.vars.push_back(SBVar(names->lookup(elem.name), info.lookup(elem.parent), elem.offset, elem.arraySize, (u8) elem.format.getValue(), (u8) elem.flags.getValue()));
		else 
			file.structs.push_back(SBStruct(names->lookup(elem.name), info.lookup(elem.parent), elem.offset, (u16) elem.arraySize, (u8) elem.flags.getValue(), elem.length));

	}

	file.size = (u32)(sizeof(SBHeader) + sizeof(SBStruct) * file.structs.size() + sizeof(SBVar) * file.vars.size());

	file.header = {
		{ 'o', 'i', 'S', 'B' },
		(u8)SBHeaderVersion::v0_1,
		(u8)((info.type.getValue() - 1U) | (u32)SBHeaderFlag::IS_ALLOCATED),
		(u16) 0U,

		(u16) file.structs.size(),
		(u16) file.vars.size(),

		info.size
	};

	return file;
}

Buffer oiSB::write(SBFile file) {

	std::vector<SBStruct> &structs = file.structs;
	std::vector<SBVar> &vars = file.vars;

	u32 structc = (u32) structs.size();
	u32 varc = (u32) vars.size();

	u32 structsiz = (u32)(sizeof(SBStruct) * structc);
	u32 varsiz = (u32)(sizeof(SBVar) * varc);

	Buffer buf((u32)(sizeof(SBHeader) + structsiz + varsiz));
	Buffer write = buf;

	file.size = buf.size();

	write.operator[]<SBHeader>(0) = file.header;
	write = write.offset((u32)sizeof(SBHeader));

	memcpy(write.addr(), structs.data(), structsiz);
	write = write.offset(structsiz);

	memcpy(write.addr(), vars.data(), varsiz);
	write = write.offset(varsiz);

	return buf;
}

bool oiSB::write(String path, SBFile file) {

	Buffer buf = write(file);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}