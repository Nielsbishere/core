#ifdef __WINDOWS__

#include "file/filemanager.h"
#include "types/string.h"
#include "types/buffer.h"
#include <fstream>
using namespace oi::wc;
using namespace oi;

bool FileManager::read(String file, String &s) const {

	std::ifstream str(file.toCString());

	if (!str.good())
		return false;

	s = std::string(std::istreambuf_iterator<char>(str), std::istreambuf_iterator<char>());
	return true;
}

bool FileManager::read(String where, Buffer &b) const {

	std::ifstream file(where.toCString(), std::ios::binary);

	if (!file.good())
		return false;

	u32 length = (u32)file.rdbuf()->pubseekoff(0, std::ios_base::end);

	file.seekg(0, std::ios::beg);
	b = Buffer(length);
	memset(b.addr(), 0, b.size());
	file.read((char*)b.addr(), b.size());

	return true;
}

bool FileManager::write(String file, String &s) const {

	std::ofstream str(s.toCString());

	if (!str.good())
		return false;

	str.write(s.toCString(), s.size());
	return true;
}

bool FileManager::write(String where, Buffer &b) const {

	std::ofstream file(where.toCString(), std::ios::binary);

	if (!file.good() || b.size() == 0)
		return false;

	file.write((const char*) b.addr(), b.size());
	return true;
}

#endif