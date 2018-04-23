#include "file/filemanager.h"
using namespace oi::wc;

FileManager::FileManager(void *param) : param(param) { instance = this; }
FileManager::~FileManager() { instance = nullptr; }
const FileManager *FileManager::get() { return instance; }

FileManager *FileManager::instance = nullptr;