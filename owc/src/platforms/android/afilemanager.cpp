#ifdef __ANDROID__

#include <android/asset_manager.h>
#include <sys/stat.h>
#include <cstring>
#include <errno.h>
#include <dirent.h>
#include "types/string.h"
#include "types/buffer.h"
#include "utils/log.h"
#include "platforms/generic.h"
#include "file/filemanager.h"
using namespace oi::wc;
using namespace oi;

String FileManager::getAbsolutePath(String path) const {
	if (path.startsWith("res") || path.startsWith("mod")) return String("res") + path.cutBegin(3);
	else if(path.startsWith("out")) return String(((android_app*)param)->activity->internalDataPath) + "/" + path.cutBegin(3);
	return "";
}

bool FileManager::dirExists(String path) const {

	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open folder for query");

	String apath = getAbsolutePath(path);

	if (path.startsWith("out")) {
		DIR *dir = opendir(apath.toCString());
		if (dir != nullptr) closedir(dir);
		return dir != nullptr;
	}

	AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
	AAssetDir *dir = AAssetManager_openDir(assetManager, path.toCString());
	if (dir != nullptr) AAssetDir_close(dir);
	return dir != nullptr;
}

bool FileManager::canModifyAssets() const { return false; }

bool FileManager::fileExists(String path) const {

	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open file for query");

	String apath = getAbsolutePath(path);

	if (path.startsWith("out")) {
		FILE *file = fopen(apath.toCString(), "r");
		if (file != nullptr) fclose(file);
		return file != nullptr;
	}

	AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
	AAsset *file = AAssetManager_open(assetManager, apath.toCString(), AASSET_MODE_BUFFER);
	if (file != nullptr) AAsset_close(file);
	return file != nullptr;
}

bool FileManager::mkdir(String path) const {

	if (!validate(path, FileAccess::WRITE)) return Log::error("Couldn't get mkdir permissions");

	path = getAbsolutePath(path);

	std::vector<String> split = path.split("/");
	String current;

	for (String &s : split) {

		if (current == "") current = "/";
		else if (current == "/") current = current + s;
		else current = current + "/" + s;

		if (current != "" && ::mkdir(current.toCString(), ACCESSPERMS) < 0) {

			String err = strerror(errno);
			if (err == "File exists") continue;

			Log::error(err);
			return Log::error(String("Couldn't mkdir \"") + current + "\"");
		}
	}

	return true;
}

void resizeType(String &s, u32 len) { s = String(len, '\0'); }
void resizeType(Buffer &b, u32 len) { b = Buffer(len); }

void *addrType(Buffer b) { return b.addr(); }
void *addrType(String &s) { return (void*) s.toCString(); }

template<typename T>
bool read(String path, T &t, void *param, const FileManager *fm) {

	if (!fm->validate(path, FileAccess::READ)) return Log::error("Couldn't open file for read");

	String apath = fm->getAbsolutePath(path);

	if (apath.startsWith("res")) {

		AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;

		AAsset *file = AAssetManager_open(assetManager, apath.toCString(), AASSET_MODE_BUFFER);

		if (file == nullptr)
			return Log::error(String("Couldn't read from file ") + path);

		u32 size = (u32) AAsset_getLength(file);
		resizeType(t, size);
		AAsset_read(file, addrType(t), size);
		AAsset_close(file);

		return true;

	}

	FILE *file = fopen(apath.toCString(), "r");

	if (file == nullptr)
		return Log::error(String("Couldn't read from file ") + path);

	fseek(file, 0, SEEK_END);
	u32 size = (u32) ftell(file);
	fseek(file, 0, SEEK_SET);
	resizeType(t, size);
	fread(addrType(t), 1, size, file);
	fclose(file);

	return true;

}

template<typename T>
bool write(String path, T &t, const FileManager *fm) {

	if (!fm->validate(path, FileAccess::WRITE)) return Log::error("Couldn't open file for write");

	String apath = fm->getAbsolutePath(path);

	if (!fm->mkdir(path.getPath()))
		return Log::error("Couldn't mkdir");

	FILE *file = fopen(apath.toCString(), "w");

	if (file == nullptr) {
		Log::error(strerror(errno));
		return Log::error(String("Couldn't write string to file ") + apath);
	}

	fwrite(addrType(t), 1, t.size(), file);
	fclose(file);
	return true;

}

bool FileManager::read(String path, String &s) const { return ::read(path, s, param, this); }
bool FileManager::read(String path, Buffer &b) const { return ::read(path, b, param, this); }

bool FileManager::write(String path, String &s) const { return ::write(path, s, this); }
bool FileManager::write(String path, Buffer b) const { return ::write(path, b, this); }

bool FileManager::foreachFile(String path, FileCallback callback) const {

	if (!validate(path, FileAccess::QUERY)) return Log::error("Couldn't open folder for query");
	if (!dirExists(path)) return Log::error("Couldn't find the specified folder");

	String apath = getAbsolutePath(path);

	if (path.startsWith("out")) {

		DIR *dir = opendir(apath.toCString());
		struct dirent *subdir;

		while ((subdir = readdir(dir)) != NULL) {

			if (subdir->d_type != DT_DIR && subdir->d_type != DT_REG)	//Only allow actual files
				continue;

			String fileName = subdir->d_name;

			if (fileName == "." || fileName == "..")
				continue;

			String filePath = path + "/" + fileName;
			bool isDir = subdir->d_type == DT_DIR;

			struct stat attr;
			stat(getAbsolutePath(filePath).toCString(), &attr);

			u64 fileSize = isDir ? 0 : (u64) attr.st_size;

			FileInfo info = FileInfo(isDir, filePath, attr.st_mtime, fileSize);

			if (callback(info))
				break;

		}

		closedir(dir);
		return true;

	}

	AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
	AAssetDir *dir = AAssetManager_openDir(assetManager, path.toCString());

	const char *name = nullptr;

	//TODO: Loop through directories

	while ((name = AAssetDir_getNextFileName(dir)) != nullptr) {

		String filePath = apath + "/" + name;

		AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
		AAsset *asset = AAssetManager_open(assetManager, filePath.toCString(), AASSET_MODE_STREAMING);

		bool isFolder = asset == nullptr;

		u64 size = 0;

		if (!isFolder) {
			size = AAsset_getLength64(asset);
			AAsset_close(asset);
		}

		FileInfo info = FileInfo(isFolder, filePath, 0, size);

		if (callback(info))
			break;

	}

	AAssetDir_close(dir);
	return true;
}

FileInfo FileManager::getFile(String path) const {

	if (!validate(path, FileAccess::QUERY)) { Log::error("Couldn't open file for query"); return {}; }

	bool isFolder = dirExists(path);

	if (!isFolder && !fileExists(path)) { Log::error("Couldn't find the specified file"); return {}; }

	String apath = getAbsolutePath(path);

	if (apath.startsWith("res")) {

		u64 size = 0;

		if (!isFolder) {
			AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
			AAsset *asset = AAssetManager_open(assetManager, path.toCString(), AASSET_MODE_STREAMING);
			size = AAsset_getLength64(asset);
			AAsset_close(asset);
		}

		return FileInfo(isFolder, path, 0, size);
	}

	struct stat attr;
	stat(apath.toCString(), &attr);

	return FileInfo(isFolder, path, attr.st_mtime, attr.st_size);

}

#endif