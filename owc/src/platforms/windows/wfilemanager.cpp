#ifdef __WINDOWS__

#define _CRT_SECURE_NO_WARNINGS
#include "file/filemanager.h"
#include "types/string.h"
#include "types/buffer.h"
#include <utils/log.h>
#include <fstream>
#include <Windows.h>
using namespace oi::wc;
using namespace oi;

bool openFile(String file, std::ifstream &in) {

	if (file.contains("../"))
		return Log::error("Couldn't open file; please use direct paths only");

	if (!file.startsWith("res/") && !file.startsWith("out/"))
		return Log::error("Couldn't open file; path has to start with res/ or out/");

	in = std::ifstream(file.toCString(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

bool openFile(String file, std::ofstream &in) {

	if (file.contains("../"))
		return Log::error("Couldn't open file; please use direct paths only");

	if (!file.startsWith("out/"))
		return Log::error("Couldn't open file; path has to start with out/");

	in = std::ofstream(file.toCString(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

bool FileManager::mkdir(String path) const {

	std::vector<String> split = path.split("/");
	String current;

	for (String &s : split) {

		if (current == "") current = s;
		else current = current + "/" + s;

		if (current != "" && CreateDirectory(getAbsolutePath(current).toCString(), NULL) == 0) {

			DWORD error = GetLastError();
			if (error == ERROR_ALREADY_EXISTS) continue;

			return Log::error(String("Couldn't mkdir ") + current + "(" + (u64)error + ")");
		}
	}

	return true;
}

String getSystemPath() {

	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);

	return String(buffer).replace("\\", "/").getPath();
}

String FileManager::getAbsolutePath(String path) const {

	static const String apath = getSystemPath();

	return apath + (path == "" ? "" : String("/") + path);

}

bool FileManager::dirExists(String path) const {
	return GetFileAttributes(path.toCString()) == FILE_ATTRIBUTE_DIRECTORY;
}

bool FileManager::fileExists(String path) const {
	FILE *file = fopen(path.toCString(), "r");
	if (file != nullptr) fclose(file);
	return file != nullptr;
}

bool FileManager::read(String file, String &s) const {

	std::ifstream in;
	if (!openFile(file, in)) return Log::error("Couldn't open file for read");

	s = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	in.close();
	return true;
}

bool FileManager::read(String file, Buffer &b) const {

	std::ifstream in;
	if (!openFile(file, in)) return Log::error("Couldn't open file for read");

	u32 length = (u32) in.rdbuf()->pubseekoff(0, std::ios_base::end);

	in.seekg(0, std::ios::beg);
	b = Buffer(length);
	memset(b.addr(), 0, b.size());
	in.read((char*)b.addr(), b.size());
	in.close();

	return true;
}

bool FileManager::write(String file, String &s) const {

	if (!mkdir(file.getPath())) return Log::error("Can't write to file; mkdir failed");

	std::ofstream out;
	if(!openFile(file, out)) return Log::error("Couldn't open file for write");

	out << s.toCString();
	out.close();
	return true;
}

bool FileManager::write(String file, Buffer b) const {

	if (!mkdir(file.getPath())) return Log::error("Can't write to file; mkdir failed");

	std::ofstream out;
	if (!openFile(file, out)) return Log::error("Couldn't open file for write");

	out.write((const char*) b.addr(), b.size());
	out.close();
	return true;
}

#endif