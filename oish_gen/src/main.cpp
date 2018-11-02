#include "file/filemanager.h"
#include "graphics/format/oish.h"
#include "graphics/helper/spvhelper.h"

using namespace oi;
using namespace oi::gc;
using namespace oi::wc;

int main() {

	FileManager fm(nullptr);

	std::unordered_map<String, std::vector<String>> shaders;

	fm.foreachFileRecurse("mod/shaders", [&](FileInfo info) -> bool { 

		String ext = info.name.getExtension();
		String file = info.name.getFilePath();

		if (ext == "hlsl" || ext == "glsl") {

			ext = file.fromLast(".");

			if (ext == file)						//It isn't a source file, it is just an include file
				return false;

			file = file.untilLast(".");

		}

		ShaderStageType type = SpvHelper::pickType(ext);

		if (type == ShaderStageType::Undefined)		//It isn't a shader type
			return false;

		shaders[file].push_back(info.name);
		return false; 
	});

	for (auto &elem : shaders) {

		String target = elem.first + ".oiSH";
		FileInfo targ;

		bool shouldModify = false;

		if (fm.fileExists(target)) targ = fm.getFile(target);
		else shouldModify = true;

		ShaderSourceType sourceType = elem.second[0].endsWith(".hlsl") ? ShaderSourceType::HLSL : ShaderSourceType::GLSL;

		for (String &str : elem.second) {

			if (str.endsWith(".hlsl") && sourceType != ShaderSourceType::HLSL)
				return 1 + (int)Log::error("Invalid shader; please don't mix HLSL and GLSL shaders");

			if (!shouldModify) {
				FileInfo temp = fm.getFile(str);
				shouldModify = targ.modificationTime < temp.modificationTime;
			}

		}

		if (!shouldModify) {
			Log::println(target + " is already up-to-date");
			continue;
		}

		ShaderSource source(elem.first.getFile(), elem.second, sourceType);

		SHFile info = oiSH::convert(source);
		if (info.bytecode.size() == 0) 
			return 1 + (int) Log::error("Couldn't compile shaders");

		if(!oiSH::write(target, info))
			return 1 + (int)Log::error("Couldn't write oiSH file");

		Log::println(target + " has been updated successfully");

	}

	return 0;

}