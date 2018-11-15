#include "file/filemanager.h"
#include "graphics/helper/bakemanager.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

int main(int argc, char *argv[]) {
	bool stripDebug = argc >= 2 && String(argv[1]) == "-strip_debug_info";
	FileManager fm(nullptr);
	BakeManager manager(stripDebug);
	return manager.run();
}