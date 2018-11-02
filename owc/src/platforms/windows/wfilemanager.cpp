#ifdef __WINDOWS__

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <Windows.h>
#include "types/string.h"
#include "types/buffer.h"
#include "utils/log.h"
#include "file/filemanager.h"
using namespace oi::wc;
using namespace oi;

bool openFile(String file, std::ifstream &in) {

	file = FileManager::get()->getAbsolutePath(file);

	in = std::ifstream(file.toCString(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

bool openFile(String file, std::ofstream &in) {

	file = FileManager::get()->getAbsolutePath(file);

	in = std::ofstream(file.toCString(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

bool FileManager::mkdir(String path) const {

	if (!validate(path, FileAccess::WRITE)) return Log::error("Mkdir requires write access");

	std::vector<String> split = path.split("/");
	String current;

	for (String &s : split) {

		if (current == "") current = s;
		else current = current + "/" + s;

		if (current != "" && CreateDirectoryA(getAbsolutePath(current).toCString(), NULL) == 0) {

			DWORD error = GetLastError();
			if (error == ERROR_ALREADY_EXISTS) continue;

			return Log::error(String("Couldn't mkdir ") + current + "(" + (u32) error + ")");
		}
	}

	return true;
}

String FileManager::getAbsolutePath(String path) const {
	return (path == "" ? "" : (path.startsWith("mod") ? String("./res") + path.cutBegin(3) : String("./") + path));

}

bool FileManager::dirExists(String path) const {
	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open folder for query");
	return GetFileAttributesA(getAbsolutePath(path).toCString()) & FILE_ATTRIBUTE_DIRECTORY;
}

bool FileManager::canModifyAssets() const { return true; }

bool FileManager::fileExists(String path) const {
	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open file for query");
	FILE *file = fopen(getAbsolutePath(path).toCString(), "r");
	if (file != nullptr) fclose(file);
	return file != nullptr;
}

bool FileManager::read(String file, String &s) const {

	if (!validate(file, FileAccess::READ)) return Log::error("Couldn't open file for read");

	std::ifstream in;
	if (!openFile(file, in)) return Log::error("Couldn't open file for read");

	s = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	in.close();
	return true;
}

bool FileManager::read(String file, Buffer &b) const {

	if (!validate(file, FileAccess::READ)) return Log::error("Couldn't open file for read");

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

	if (!validate(file, FileAccess::WRITE)) return Log::error("Couldn't open file for write");
	if (!mkdir(file.getPath())) return Log::error("Can't write to file; mkdir failed");

	std::ofstream out;
	if(!openFile(file, out)) return Log::error("Couldn't open file for write");

	out << s.toCString();
	out.close();
	return true;
}

bool FileManager::write(String file, Buffer b) const {

	if (!validate(file, FileAccess::WRITE)) return Log::error("Couldn't open file for write");
	if (!mkdir(file.getPath())) return Log::error("Can't write to file; mkdir failed");

	std::ofstream out;
	if (!openFile(file, out)) return Log::error("Couldn't open file for write");

	out.write((const char*) b.addr(), b.size());
	out.close();
	return true;
}

bool FileManager::foreachFile(String path, FileCallback callback) const {

	if(!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open folder for query");
	if (!dirExists(path)) return Log::error("Couldn't find the specified folder");

	String startPath = path + "/";
	path = getAbsolutePath(path) + "/*";

	WIN32_FIND_DATA data;
	HANDLE file = FindFirstFileA(path.toCString(), &data);
	bool first = true;

	if (file == INVALID_HANDLE_VALUE) return Log::error("Couldn't find directory");

	while (first || FindNextFileA(file, &data)) {

		first = false;

		bool isDir = data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		u64 fileSize = isDir ? 0 : ((u64)data.nFileSizeHigh << 32) | data.nFileSizeLow;

		ULARGE_INTEGER ull;
		ull.LowPart = data.ftLastWriteTime.dwLowDateTime;
		ull.HighPart = data.ftLastWriteTime.dwHighDateTime;

		time_t modificationTime = ull.QuadPart / 10000000ULL - 11644473600ULL;	//Convert to time_t

		String fileName = data.cFileName;

		if (fileName == "." || fileName == "..")
			continue;

		FileInfo info = FileInfo(isDir, startPath + fileName, modificationTime, fileSize);

		if (callback(info))
			break;

	}

	return true;

}

FileInfo FileManager::getFile(String path) const {

	if (!validate(path, FileAccess::QUERY)) { Log::error("Couldn't open file for query"); return {}; }
	
	bool isFolder = dirExists(path);
	
	if (!isFolder && !fileExists(path)) { Log::error("Couldn't find the specified file"); return {}; }

	String apath = getAbsolutePath(path);

	struct _stat64 attr;
	_stat64(apath.toCString(), &attr);

	return FileInfo(isFolder, path, attr.st_mtime, attr.st_size);

}

#endif