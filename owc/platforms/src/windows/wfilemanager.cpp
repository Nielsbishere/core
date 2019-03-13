#ifdef __WINDOWS__

#include <fstream>
#include <algorithm>
#include "types/string.h"
#include "types/buffer.h"
#include "utils/log.h"
#include "file/filemanager.h"
#include "windows/windows.h"
using namespace oi::wc;
using namespace oi;

bool openFile(String file, std::ifstream &in) {

	file = FileManager::get()->getAbsolutePath(file);

	in = std::ifstream(file.begin(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

bool openFile(String file, std::ofstream &in) {

	file = FileManager::get()->getAbsolutePath(file);

	in = std::ofstream(file.begin(), std::ios::binary);

	if (!in.good())
		return Log::error(String("Couldn't open file: ") + file);

	return true;
}

namespace oi {

	namespace wc {

		struct FileManagerExt {

			static BOOL enumerateFiles(HMODULE, LPCSTR, LPSTR name, LONG_PTR fileManager) {

				FileManager *fm = (FileManager*)fileManager;
				String sname = String(name).toLowerCase();
				Array<String> parts = sname.split("/");
				String currName = "";
				u32 last = 0;

				for (u32 i = 0, j = (u32) parts.size() - 1; i < j; ++i) {

					currName += String(currName != "" ? "/" : "") + parts[i];

					auto it = std::find(fm->dirs.begin(), fm->dirs.end(), currName);

					if (it == fm->dirs.end()){
						fm->dirs.push_back(currName);
						last = (u32) fm->dirs.size();
					} else
						last = (u32)(it - fm->dirs.begin());

				}

				fm->files.push_back({ sname, last });
				return true;
			}

		};

	}

}

void FileManager::init() {

	EnumResourceNamesA(nullptr, RT_RCDATA, FileManagerExt::enumerateFiles, (LONG_PTR) this);

}

bool FileManager::mkdir(String path) const {

	if (!validate(path, FileAccess::WRITE)) return Log::error("Mkdir requires write access");

	Array<String> split = path.split("/");
	String current;

	for (String &s : split) {

		if (current == "") current = s;
		else current = current + "/" + s;

		if (current != "" && CreateDirectoryA(getAbsolutePath(current).begin(), NULL) == 0) {

			DWORD error = GetLastError();
			if (error == ERROR_ALREADY_EXISTS) continue;

			return Log::error(String("Couldn't mkdir ") + current + "(" + (u32) error + ")");
		}
	}

	return true;
}

String FileManager::getAbsolutePath(String path) const {
	return (path == "" ? "" : (path.startsWith("mod") ? String("res") + path.cutBegin(3) : path));

}

bool FileManager::dirExists(String path) const {
	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open folder for query");
	if (path.startsWith("res")) return std::find(dirs.begin(), dirs.end(), path) != dirs.end();
	return GetFileAttributesA(getAbsolutePath(path).begin()) & FILE_ATTRIBUTE_DIRECTORY;
}

bool FileManager::canModifyAssets() const { return true; }

bool FileManager::fileExists(String path) const {
	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open file for query");
	if (path.startsWith("res")) return std::find_if(files.begin(), files.end(), [path](const ParentedFileInfo &info) -> bool { return info.name == path.toLowerCase(); }) != files.end();
	FILE *file = fopen(getAbsolutePath(path).begin(), "r");
	if (file != nullptr) fclose(file);
	return file != nullptr;
}

bool FileManager::read(String file, String &s) const {

	if (!validate(file, FileAccess::READ)) return Log::error("Couldn't open file for read");

	if (file.startsWith("res")) {

		HRSRC data = FindResourceA(nullptr, file.begin(), RT_RCDATA);

		if (data == nullptr)
			return Log::error("Couldn't find resource");

		u32 size = (u32) SizeofResource(nullptr, data);
		HGLOBAL handle = LoadResource(nullptr, data);

		if (handle == nullptr)
			return Log::error("Couldn't load resource");

		char *dat = (char*) LockResource(handle);

		s = String(size, dat);
		UnlockResource(handle);
		FreeResource(handle);
		return true;
	}

	std::ifstream in;
	if (!openFile(file, in)) return Log::error("Couldn't open file for read");

	u32 length = (u32)in.rdbuf()->pubseekoff(0, std::ios_base::end);

	in.seekg(0, std::ios::beg);
	s = String(length);
	in.read(s.begin(), length);
	in.close();
	return true;
}

bool FileManager::read(String file, Buffer &b) const {

	if (!validate(file, FileAccess::READ)) return Log::error("Couldn't open file for read");

	if (file.startsWith("res")) {

		HRSRC data = FindResourceA(nullptr, file.begin(), RT_RCDATA);

		if (data == nullptr)
			return Log::error("Couldn't find resource");

		u32 size = (u32)SizeofResource(nullptr, data);
		HGLOBAL handle = LoadResource(nullptr, data);

		if (handle == nullptr)
			return Log::error("Couldn't load resource");

		u8 *dat = (u8*) LockResource(handle);

		b = Buffer(dat, size);
		UnlockResource(handle);
		FreeResource(handle);
		return true;
	}

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

	out << s.begin();
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

	if (path.startsWith("res")) {

		u32 dirId = u32(std::find(dirs.begin(), dirs.end(), path) - dirs.begin());

		for (auto &file : files)
			if (file.dirId == dirId)
				callback(getFile(file.name));

		path += "/";

		for (auto &dir : dirs)
			if (dir.startsWith(path))
				callback(getFile(dir));

		return true;
	}

	String startPath = path + "/";
	path = getAbsolutePath(path) + "/*";

	WIN32_FIND_DATA data;
	HANDLE file = FindFirstFileA(path.begin(), &data);
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
	memset(&attr, 0, sizeof(attr));

	if(!path.startsWith("res"))
		_stat64(apath.begin(), &attr);
	else if (!isFolder) {
		if (HRSRC data = FindResourceA(nullptr, path.begin(), RT_RCDATA))
			attr.st_size = (u64)SizeofResource(nullptr, data);
	}

	return FileInfo(isFolder, path, attr.st_mtime, attr.st_size);

}

#endif