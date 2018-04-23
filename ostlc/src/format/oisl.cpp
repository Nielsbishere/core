#include "format/oisl.h"
#include "types/buffer.h"
#include <cmath>
using namespace oi;

bool oiSL::read(String path, SLFile &file) {

	Buffer buf = Buffer::readFile(path);

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

	u32 names = header.names * sizeof(SLString);

	if (buf.size() < header.keys + names)
		return Log::error("Invalid oiSL file");

	SLHeaderVersion v(header.version);

	switch (v.getValue()) {

	case SLHeaderVersion::v1.value:
		goto v1;

	default:
		return Log::error("Invalid oiSL (header) file");

	}

v1:
	{

		if (header.flags & (u8) SLHeaderFlags::USE_DEFAULT)
			file.keyset = String::getDefaultCharset();
		else {

			file.keyset = String(header.keys, ' ');

			for (u8 i = 0; i < header.keys; ++i)
				file.keyset[i] = (u8)(~buf.addr()[i]);

			buf = buf.offset(header.keys);
		}

		std::vector<SLString> strings(header.names);
		std::vector<String> &str = file.names = std::vector<String>(header.names);

		strings.assign((SLString*)buf.addr(), (SLString*)(buf.addr() + names));
		buf = buf.offset(names);

		String decoded = String::decode(buf, file.keyset, header.perChar, header.length);
		buf = buf.offset((u32) std::ceil(header.length * header.perChar / 8.f));

		for (u32 i = 0; i < header.names; ++i) {
			SLString &s = strings[i];
			str[i] = String((char*)decoded.toCString() + s.start, s.size);
		}

		goto end;

	}

end:

	file.size = (u32)(buf.addr() - start.addr());

	Log::println(String("Successfully loaded oiSL file with version ") + v.getName());

	return true;
}

Buffer oiSL::write(String keyset, std::vector<String> names) {

	bool useDefault = keyset == String::getDefaultCharset();

	std::vector<SLString> strings(names.size());
	
	String toEncode;

	for (u32 i = 0; i < (u32) names.size(); ++i) {

		String &s = names[i];

		strings[i] = { (u8)s.size(), (u16)toEncode.size() };
		toEncode += s;
	}

	u32 keycount = keyset.size() == 1 ? 1 : keyset.size() - 1;
	u8 perChar = (u8) (std::floor(std::log2(keycount)) + 1);

	Buffer buf = toEncode.encode(keyset, perChar);

	Buffer toFile((u32) (sizeof(SLHeader) + (!useDefault ? keyset.size() : 0) + names.size() * sizeof(SLString) + buf.size()));
	Buffer write = toFile;

	write.operator[]<SLHeader>(0) = {
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
		write.copy(Buffer::construct((u8*)keyset.toCString(), (u32)keyset.size()));
		write = write.offset((u32)keyset.size());
	}

	u32 string = (u32)(strings.size() * sizeof(SLString));

	memcpy(write.addr(), strings.data(), string);
	write = write.offset(string);

	write.copy(buf);

	buf.deconstruct();

	return toFile;
}

bool oiSL::write(String path, String keyset, std::vector<String> names) {

	Buffer buf = write(keyset, names);

	if (!buf.writeFile(path)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}