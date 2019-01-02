#include "file/filemanager.h"
#include "types/string.h"
#include "utils/log.h"
using namespace oi::wc;
using namespace oi;

FileManager::FileManager(void *param) : param(param) { instance = this; init(); }
FileManager::~FileManager() { instance = nullptr; }
const FileManager *FileManager::get() { return instance; }

FileManager *FileManager::instance = nullptr;

bool FileManager::exists(String path) const {
	return dirExists(path) || fileExists(path);
}

bool FileManager::validate(String path, FileAccess writeAccess) const {

	if (path.contains("../") || path.contains("./") || path.contains("\\"))
		return Log::error("File path can only be direct");

	String prefix = path.untilFirst("/");

	if (prefix != "res" && prefix != "out" && prefix != "mod")
		return Log::error("File path has to start with a valid prefix");

	if (writeAccess <= FileAccess::READ || prefix == "out" || (prefix == "mod" && canModifyAssets())) {
		if (writeAccess == FileAccess::READ && !fileExists(path))
			return Log::error(String("Couldn't open file for read; it doesn't exist (") + path + ")");
		
		return true;
	}

	return Log::error("File path couldn't give the required access");

}

bool FileManager::foreachFileRecurse(String path, FileCallback callback) const {

	return foreachFile(path, [callback](FileInfo info) -> bool {

		if (info.isFolder)
			FileManager::get()->foreachFileRecurse(info.name, callback);

		if (callback(info))
			return true;

		return false; 
	});

}