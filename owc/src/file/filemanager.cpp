#include "file/filemanager.h"
#include <types/string.h>
using namespace oi::wc;
using namespace oi;

FileManager::FileManager(void *param) : param(param) { instance = this; }
FileManager::~FileManager() { instance = nullptr; }
const FileManager *FileManager::get() { return instance; }

FileManager *FileManager::instance = nullptr;

bool FileManager::exists(String path) const {
	return dirExists(path) || fileExists(path);
}