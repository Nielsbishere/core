#ifdef __ANDROID__

#include "file/filemanager.h"
#include <types/string.h>
#include <types/buffer.h>
#include <utils/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
using namespace oi::wc;
using namespace oi;

bool FileManager::read(String path, String &s) const {

	AAssetManager *assetManager = ((android_app*) param)->activity->assetManager;

	AAsset *file = AAssetManager_open(assetManager, path.toCString(), AASSET_MODE_BUFFER);

	if (file == nullptr)
		return Log::error(String("Couldn't read string from file \"") + path);

	s = String((u32) AAsset_getLength(file) + 1U, '\0');
	AAsset_read(file, (void*) s.toCString(), s.size() - 1U);

	return true;
}

bool FileManager::read(String path, Buffer &b) const {

	AAssetManager *assetManager = ((android_app*)param)->activity->assetManager;

	AAsset *file = AAssetManager_open(assetManager, path.toCString(), AASSET_MODE_BUFFER);

	if (file == nullptr)
		return Log::error(String("Couldn't read buffer from file \"") + path);

	b = Buffer((u32) AAsset_getLength(file));
	AAsset_read(file, b.addr(), b.size());

	return true;
}

bool FileManager::write(String path, String &s) const {

	String p = String(((android_app*)param)->activity->internalDataPath) + "/" + path;


}

bool FileManager::write(String path, Buffer &b) const {

	String p = String(((android_app*)param)->activity->internalDataPath) + "/" + path;

	return Log::error("FileManager::write not implemented for Android");
}

#endif