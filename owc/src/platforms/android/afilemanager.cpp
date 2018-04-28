#ifdef __ANDROID__

#include "platforms/generic.h"
#include "file/filemanager.h"
#include <types/string.h>
#include <types/buffer.h>
#include <utils/log.h>
#include <android/asset_manager.h>
#include <sys/stat.h>
#include <cstring>
#include <errno.h>
#include <dirent.h>
using namespace oi::wc;
using namespace oi;

String FileManager::getAbsolutePath(String path) const {
	if (path.startsWith("res/")) return path;
	else if(path.startsWith("out/")) return String(((android_app*)param)->activity->internalDataPath) + "/" + path.cutBegin(4);
	return "";
}

bool FileManager::dirExists(String path) const {

	String apath = getAbsolutePath(path);
	
	if (path.startsWith("out/")) {
		DIR *dir = opendir(apath.toCString());
		if (dir != nullptr) closedir(dir);
		return dir != nullptr;
	} else if(path.startsWith("res/")){
		AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
		AAssetDir *dir = AAssetManager_openDir(assetManager, path.toCString());
		if (dir != nullptr) AAssetDir_close(dir);
		return dir != nullptr;
	}

	return false;
}

bool FileManager::fileExists(String path) const {

	String apath = getAbsolutePath(path);

	if (path.startsWith("out/")) {
		FILE *file = fopen(apath.toCString(), "r");
		if (file != nullptr) fclose(file);
		return file != nullptr;
	} else if(path.startsWith("res/")) {
		AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;
		AAsset *file = AAssetManager_open(assetManager, apath.toCString(), AASSET_MODE_BUFFER);
		if (file != nullptr) AAsset_close(file);
		return file != nullptr;
	}

	return false;
}

bool FileManager::mkdir(String path) const {

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

	if (path.contains("../"))
		return Log::error("Couldn't read file; Please use direct paths only");

	String apath = fm->getAbsolutePath(path);

	if (path.startsWith("res/")) {

		AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;

		AAsset *file = AAssetManager_open(assetManager, apath.toCString(), AASSET_MODE_BUFFER);

		if (file == nullptr)
			return Log::error(String("Couldn't read from file ") + path);

		u32 size = (u32) AAsset_getLength(file);
		resizeType(t, size);
		AAsset_read(file, addrType(t), size);
		AAsset_close(file);

		return true;

	} else if (path.startsWith("out/")) {

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

	return Log::error("Couldn't read from file; path has to start with res/ or out/");
}

template<typename T>
bool write(String path, T &t, const FileManager *fm) {

	if (path.contains("../"))
		return Log::error("Couldn't write file; Please use direct paths only");

	if (!path.startsWith("out/"))
		return Log::error("Couldn't write file; write path can only be out/ (res/ is read only for example)");

	String apath = fm->getAbsolutePath(path);

	if (!fm->mkdir(apath.getPath()))
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

#endif