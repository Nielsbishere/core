#include <cmath>
#include "format/oisl.h"
#include "types/buffer.h"
#include "file/filemanager.h"
#include "types/bitset.h"
using namespace oi::wc;
using namespace oi;

SLFile::SLFile() : SLFile("", {}) {}
SLFile::SLFile(const String &keyset, const Array<String> &strings) : keyset(keyset), strings(strings) {
	for (const String &s : strings)
		if (s.size() >= maxStringSize)
			Log::throwError<SLFile, 0x0>("String exceed maximum length limit");
}

u32 SLFile::lookup(const String &string) const {

	u32 i{};

	for (const String &n : strings)
		if (n == string) break;
		else ++i;

	return i;
}

u32 SLFile::add(const String &string) {

	if (string.size() >= maxStringSize)
		Log::throwError<SLFile, 0x1>("String exceed maximum length limit");

	u32 id = lookup(string);

	if (id == u32(strings.size()))
		strings = strings.pushBack(string);

	return id;

}

bool SLFile::read(String path, SLFile &file) {

	Buffer buf;
	FileManager::get()->read(path, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't open file");

	if (!read(buf, file))
		return Log::error("Couldn't read file");

	buf.deconstruct();
	return true;
}

bool SLFile::read(Buffer buf, SLFile &file) {

	Buffer start = buf;

	if (buf.size() < sizeof(SLHeader))
		return Log::error("Invalid oiSL file");

	SLHeader header = buf.operator[]<SLHeader>(0);
	buf = buf.offset((u32) sizeof(SLHeader));

	if (header.magicNumber != SLFile::magicNumber)
		return Log::error("Invalid oiSL (header) file");

	switch (header.version) {

	case SLVersion::V1:
		goto v1;

	default:
		return Log::error("Invalid oiSL (header) file");

	}

v1:
	{

		u32 names = header.strings;

		if (buf.size() < header.keys * (1 - header.useDefaultCharset) + names * sizeof(SizeType))
			return Log::error("Invalid oiSL file");

		if (header.useDefaultCharset)
			file.keyset = String::getDefaultCharset();
		else {

			file.keyset = String(header.keys, ' ');

			for (u8 i = 0; i < header.keys; ++i)
				file.keyset[i] = (u8)(~buf.addr()[i]);

			buf = buf.offset(header.keys);
		}

		Array<SizeType> strings(header.strings);
		Array<String> &str = file.strings = Array<String>(header.strings);

		memcpy(strings.begin(), buf.addr(), names);
		buf = buf.offset(names);

		size_t length = 0, perChar = file.getBitDepth();

		for (SizeType &st : strings)
			length += st;

		String decoded = String::decode(buf, file.keyset, u8(perChar), u32(length));
		buf = buf.offset((u32)std::ceil(length * perChar / 8.f));

		u32 offset = 0;

		for (u32 i = 0; i < header.strings; ++i) {
			SizeType &s = strings[i];
			str[i] = String(s, (char*)decoded.begin() + offset);
			offset += s;
		}

		goto end;

	}

end:

	if (buf.size() != 0)
		file.size = (u32)(buf.addr() - start.addr());
	else
		file.size = start.size();

	Log::println(String("Successfully loaded oiSL file with version ") + size_t(header.version) + " (" + file.size + " bytes)");

	return true;
}

Buffer SLFile::write(SLFile &file) {

	String &keyset = file.keyset;
	Array<String> &names = file.strings;

	bool useDefault = keyset == String::getDefaultCharset();

	CopyBuffer strings((u32)names.size());
	
	String toEncode;

	for (u32 i = 0; i < (u32) names.size(); ++i) {

		String &s = names[i];

		strings[i] = (u8)s.size();
		toEncode += s;
	}

	size_t keycount = keyset.size() == 1 ? 1 : keyset.size() - 1;
	u8 perChar = (u8) (std::floor(std::log2(keycount)) + 1);

	Buffer buf = toEncode.encode(keyset, perChar);

	Buffer toFile((u32) (sizeof(SLHeader) + (!useDefault ? keyset.size() : 0) + names.size() + buf.size()));
	file.size = toFile.size();

	Buffer write = toFile;

	write.operator[]<SLHeader>(0) = {
		SLFile::magicNumber,

		u16(keyset.size()),
		SLVersion::V1,
		useDefault,
		0,

		u32(names.size())
	};
	write = write.offset((u32) sizeof(SLHeader));

	if (!useDefault) {

		for (u32 i = 0, j = (u32)keyset.size(); i < j; ++i)
			write[i] = ~(u8)(keyset[i]);

		write = write.offset((u32)keyset.size());
	}

	u32 string = (u32) strings.size();

	memcpy(write.addr(), strings.begin(), string);
	write = write.offset(string);

	write.copy(buf);

	buf.deconstruct();

	return toFile;
}

bool SLFile::write(SLFile &file, String path) {

	Buffer buf = write(file);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}


size_t SLFile::getBitDepth() const {
	return size_t(std::fmax(std::ceil(std::log2(keyset.size())), 1));
}

size_t SLFile::getStringsBitCount() const {

	size_t bitDepth = getBitDepth(), encodedBits = 0;

	for (const String &str : strings)
		encodedBits += bitDepth * str.size();

	return encodedBits;
}

size_t SLFile::getStringsDataSize() const {
	return size_t(std::ceil(getStringsBitCount() / 8.0));
}