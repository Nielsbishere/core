#include <api/spv/spvhelper.h>
#include <graphics/format/oish.h>

#include <fstream>

using namespace oi;
using namespace oi::gc;

int main(int argc, char *argv[]) {

	String path, shaderName;
	std::vector<String> extensions;

	if (argc < 4) return (int) Log::error("Incorrect usage: oish_gen.exe <pathToShader> <shaderName> [shaderStage extensions]");

	path = argv[1];
	shaderName = argv[2];
	
	for (int i = 3; i < argc; ++i)
		extensions.push_back(argv[i]);

	std::unordered_map<String, CopyBuffer> spv;

	//Open the extensions' spirv and parse their data
	for (String &s : extensions) {

		std::ifstream str((path + s + ".spv").toCString(), std::ios::binary);

		if (!str.good()) return (int) Log::error("Couldn't open that file");

		u32 length = (u32)str.rdbuf()->pubseekoff(0, std::ios_base::end);

		CopyBuffer &b = spv[s] = CopyBuffer(length);
		str.seekg(0, std::ios::beg);
		str.read((char*)b.addr(), b.size());
		str.close();

	}

	//Convert to oiSH file
	SHFile shFile = oiSH::convert(ShaderSource(shaderName, spv));

	if(shFile.stringlist.names.size() == 0)
		return (int)Log::error("Couldn't convert to oiSH");

	Buffer b = oiSH::write(shFile);

	if(b.size() == 0)
		return (int) Log::error("Couldn't write oiSH");

	std::ofstream oish((path + ".oiSH").toCString(), std::ios::binary);

	if (!oish.good()) return (int)Log::error("Couldn't open that file");
	
	oish.write((char*)b.addr(), b.size());
	oish.close();

	b.deconstruct();

	Log::println(String("Successfully converted to ") + path + ".oiSH");

	return 1U;
}