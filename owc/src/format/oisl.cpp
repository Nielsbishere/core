#include <cmath>
#include "format/oisl.h"
#include "types/buffer.h"
#include "file/filemanager.h"
using namespace oi::wc;
using namespace oi;

SLFile::SLFile(String keyset, std::vector<String> names) : keyset(keyset), names(names) {}
SLFile::SLFile() : SLFile("", {}) {}

u32 SLFile::lookup(String name) {
	u32 i = 0U;
	for (String &n : names)
		if (n == name) break;
		else ++i;

	return i;
}

u32 SLFile::add(String name) {

	u32 id = lookup(name);

	if (id == names.size())
		names.push_back(name);

	return id;

}

bool oiSL::read(String path, SLFile &file) {

	Buffer buf;
	FileManager::get()->read(path, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't open file");

	if (!read(buf, file))
		return Log::error("Couldn't read file");

	buf.deconstruct();
	return true;
}

bool oiSL::read(Buffer buf, SLFile &file) {

	Buffer start = buf;

	if (buf.size() < sizeof(SLHeader))
		return Log::error("Invalid oiSL file");

	SLHeader &header = file.header = buf.operator[]<SLHeader>(0);
	buf = buf.offset((u32) sizeof(SLHeader));

	if (String(header.header, 4) != "oiSL")
		return Log::error("Invalid oiSL (header) file");

	SLHeaderVersion v(header.version);

	switch (v.getValue()) {

	case SLHeaderVersion::v1.value:
		goto v1;

	default:
		return Log::error("Invalid oiSL (header) file");

	}

v1:
	{

		u32 names = header.names;

		if (buf.size() < header.keys + names)
			return Log::error("Invalid oiSL file");

		if (header.flags & (u8) SLHeaderFlags::USE_DEFAULT)
			file.keyset = String::getDefaultCharset();
		else {

			file.keyset = String(header.keys, ' ');

			for (u8 i = 0; i < header.keys; ++i)
				file.keyset[i] = (u8)(~buf.addr()[i]);

			buf = buf.offset(header.keys);
		}

		CopyBuffer strings(header.names);
		std::vector<String> &str = file.names = std::vector<String>(header.names);

		memcpy(strings.addr(), buf.addr(), names);
		buf = buf.offset(names);

		String decoded = String::decode(buf, file.keyset, header.perChar, header.length);
		buf = buf.offset((u32) std::ceil(header.length * header.perChar / 8.f));

		u32 offset = 0;

		for (u32 i = 0; i < header.names; ++i) {
			u8 &s = strings[i];
			str[i] = String((char*)decoded.toCString() + offset, s);
			offset += s;
		}

		goto end;

	}

end:

	if(buf.size() != 0)
		file.size = (u32)(buf.addr() - start.addr());
	else
		file.size = start.size();

	Log::println(String("Successfully loaded oiSL file with version ") + v.getName() + " (" + file.size + " bytes)");

	return true;
}

Buffer oiSL::write(SLFile &file) {

	String &keyset = file.keyset;
	std::vector<String> &names = file.names;

	bool useDefault = keyset == String::getDefaultCharset();

	CopyBuffer strings((u32)names.size());
	
	String toEncode;

	for (u32 i = 0; i < (u32) names.size(); ++i) {

		String &s = names[i];

		strings[i] = (u8)s.size();
		toEncode += s;
	}

	u32 keycount = keyset.size() == 1 ? 1 : keyset.size() - 1;
	u8 perChar = (u8) (std::floor(std::log2(keycount)) + 1);

	Buffer buf = toEncode.encode(keyset, perChar);

	Buffer toFile((u32) (sizeof(SLHeader) + (!useDefault ? keyset.size() : 0) + names.size() + buf.size()));
	file.size = toFile.size();

	Buffer write = toFile;

	file.header = write.operator[]<SLHeader>(0) = {
		{ 'o', 'i', 'S', 'L' },

		(u8) SLHeaderVersion::v1,
		perChar,
		(u8)(useDefault ? 0 : keyset.size()),
		(u8)(useDefault ? SLHeaderFlags::USE_DEFAULT : SLHeaderFlags::NONE),

		(u16) names.size(),
		(u16) toEncode.size()
	};
	write = write.offset((u32) sizeof(SLHeader));

	if (!useDefault) {

		for (u32 i = 0, j = (u32)keyset.size(); i < j; ++i)
			write[i] = ~(u8)(keyset[i]);

		write = write.offset((u32)keyset.size());
	}

	u32 string = (u32) strings.size();

	memcpy(write.addr(), strings.addr(), string);
	write = write.offset(string);

	write.copy(buf);

	buf.deconstruct();

	return toFile;
}

bool oiSL::write(SLFile &file, String path) {

	Buffer buf = write(file);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}