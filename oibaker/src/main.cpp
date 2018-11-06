#include "file/filemanager.h"
#include "graphics/helper/bakemanager.h"
using namespace oi::gc;
using namespace oi::wc;

int main() {
	FileManager fm(nullptr);
	BakeManager manager;
	return manager.run();
}