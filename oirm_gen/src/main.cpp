#include <graphics/format/obj.h>
#include <graphics/format/fbx.h>
#include <utils/log.h>
#include <utils/timer.h>
#include <fstream>
using namespace oi::gc;
using namespace oi;

//This can't be handled by the FileHandler, because the FileHandler is only available runtime from an app
//The FileHandler itself is based around read and readwrite structure, which we don't care about on PC.
//FileHandler is also relative to the working directory and not absolute.
Buffer loadFileAbsolute(String path) {

	std::ifstream file(path.toCString(), std::ios::binary);

	if (!file.good()) {
		Log::error("Couldn't open that file");
		return {};
	}

	u32 length = (u32) file.rdbuf()->pubseekoff(0, std::ios_base::end);

	Buffer b = Buffer(length);
	file.seekg(0, std::ios::beg);
	file.read((char*)b.addr(), b.size());
	file.close();

	return b;

}

//This can't be handled by the FileHandler, because the FileHandler is only available runtime from an app
//The FileHandler itself is based around read and readwrite structure, which we don't care about on PC.
//FileHandler is also relative to the working directory and not absolute.
bool writeFileAbsolute(String path, Buffer buffer) {

	std::ofstream file(path.toCString(), std::ios::binary);

	if (!file.good())
		return Log::error("Couldn't open that file (for write)");

	file.write((char*)buffer.addr(), buffer.size());
	file.close();

	return true;
}

//This project is the conversion from obj/fbx to oiRM file format.
//It is written for desktop only; since those are the ones building the projects.
//In an Android App, you're not able to write into resources, so all of that should be handled beforehand.
int main(int argv, char *argc[]) {

	if (argv < 2)
		return (int) Log::error("Syntax: oirm_gen.exe \"modelPath\" (compression; y/n)");

	Timer t;

	String path = argc[1];
	bool compression = argv < 3 ? true : (String(argc[2]) == "y");

	bool isValid = path.endsWithIgnoreCase(".obj") || path.endsWithIgnoreCase(".fbx");

	if(!isValid)
		return (int) Log::error(String("Syntax: oirm_gen.exe \"modelPath\"; \"") + path + "\" is not a valid model path");

	Buffer buf = loadFileAbsolute(path);

	if(buf.size() == 0)
		return (int) Log::error(String("Couldn't load \"") + path + "\"");

	std::vector<u32> loc = path.find('.');
	String base = path.cutEnd(loc[loc.size() - 1]);

	int res = 1;

	if (path.endsWithIgnoreCase(".obj")) {

		Buffer model = Obj::convert(buf, compression);

		if (model.size() == 0)
			res = (int) Log::error(String("Couldn't convert \"") + path + "\"");
		else {
			res = (int) writeFileAbsolute(base + ".oiRM", model);
			model.deconstruct();
			if(res) Log::println(String("Successfully converted \"") + path + "\" to \"" + base + ".oiRM\"");
		}

	} else if (path.endsWithIgnoreCase(".fbx")) {

		String fname = path.fromLast("\\").untilLast(".");

		auto models = Fbx::convertMeshes(buf, compression);

		for (auto &elem : models) {

			if (elem.first.equalsIgnoreCase(fname)) {
				res = (int)writeFileAbsolute(base + ".oiRM", elem.second);
				if(res) Log::println(String("Successfully converted \"") + path + "\" to \"" + base + ".oiRM\"");
			} else {
				res = (int)writeFileAbsolute(base + "." + elem.first + ".oiRM", elem.second);
				if(res) Log::println(String("Successfully converted \"") + path + "\" to \"" + base + "." + elem.first + + ".oiRM\"");
			}

			elem.second.deconstruct();
		}

	}

	buf.deconstruct();
	t.print();

	return res;
}