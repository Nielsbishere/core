#include "file/filemanager.h"
#include "graphics/format/obj.h"
#include "graphics/format/fbx.h"
#include "graphics/format/oish.h"
#include "graphics/helper/spvhelper.h"
#include "graphics/helper/bakemanager.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

bool BakeManager::bakeModel(BakedFile &file, bool) {

	if (file.extension == "obj") {

		file.outputs.resize(1);
		file.outputs[0] = file.file + ".oiRM";

		if (!Obj::convert(file.file + "." + file.extension, file.outputs[0], true))
			return Log::error(file.inputs[0] + " couldn't bake obj model");

		return true;

	}

	std::unordered_map<String, Buffer> res;

	if ((res = Fbx::convertMeshes(file.inputs[0], true)).size() == 0)
		return Log::error(file.inputs[0] + " couldn't bake fbx model");

	file.outputs.resize(res.size());

	bool complete = true;

	u32 i = 0;
	for (auto &elem : res) {

		String current = file.file + "." + elem.first + ".oiRM";

		if (res.size() == 1 || elem.first == "" || elem.first == file.file.getFile())
			current = file.file + ".oiRM";

		file.outputs[i] = current;

		if (!FileManager::get()->write(current, elem.second)) {
			complete = false;
			Log::error(current + " couldn't bake fbx model");
		}

		elem.second.deconstruct();
		++i;
	}

	return complete;

}


bool BakeManager::bakeShader(BakedFile &file, bool stripDebug) {

	file.outputs.resize(1);
	file.outputs[0] = file.file + ".oiSH";

	ShaderSource source(file.file.getFileName(), file.inputs, file.extension == "hlsl" ? ShaderSourceType::HLSL : ShaderSourceType::GLSL);

	SHFile info = oiSH::convert(source, file.dependencies, stripDebug);
	if (info.bytecode.size() == 0)
		return Log::error(file.outputs[0] + " couldn't compile shader");

	if (!oiSH::write(file.outputs[0], info))
		return Log::error(file.outputs[0] + " couldn't write oiSH file");

	return true;

}

BakeManager::BakeManager(bool stripDebug, String file) : location(file), stripDebug(stripDebug), bakeOptions({

	BakeOption(
		"Models",
		"mod/models",
		{
			{ "fbx", { "fbx" } },
			{ "obj", { "obj" } }
		},
		BakeManager::bakeModel
	),

	BakeOption(
		"Shaders",
		"mod/shaders",
		{
			{ "glsl", { "vert", "frag", "comp", "geom", "tese", "tesc" } },
			{ "glsl", { "vert.glsl", "frag.glsl", "comp.glsl", "geom.glsl", "tese.glsl", "tesc.glsl" } },
			{ "hlsl", { "vert.hlsl", "frag.hlsl", "comp.hlsl", "geom.hlsl", "tese.hlsl", "tesc.glsl" } }
		},
		BakeManager::bakeShader
	)

	}) {
	load();
}

void BakeManager::load() {

	Buffer buf;

	if (FileManager::get()->fileExists(location) && FileManager::get()->read(location, buf)) {

		BMHeader header;

		Buffer write = buf;

		if (!write.read(header))
			Log::throwError<BakeManager, 0x0>("Invalid BakeManager file");

		if (memcmp(header.header, "oiBM", 4) != 0)
			Log::throwError<BakeManager, 0x1>("Invalid BakeManager file header");

		//The file format isn't valid anymore; rebake
		if (header.version != BMHeader::getGlobalVersion())
			return;

		//Different flags; rebake
		if (header.flags != makeFlags())
			return;

		file.header = header;

		std::vector<BMBakedFile> bakedFiles;

		if (!write.read(bakedFiles, header.files))
			Log::throwError<BakeManager, 0x2>("Invalid BakeManager file size");

		u32 ushorts = 0;

		for (BMBakedFile &bmbf : bakedFiles)
			ushorts += bmbf.inputs + bmbf.outputs + bmbf.dependencies;

		std::vector<u16> bakedInfo;

		if (!write.read(bakedInfo, ushorts))
			Log::throwError<BakeManager, 0x3>("Invalid BakeManager's baked file size");

		u16 *bakedInfop = bakedInfo.data();

		if (!oiSL::read(write, file.strings))
			Log::throwError<BakeManager, 0x4>("Invalid BakeManager strings");

		file.files.resize(header.files);
		u32 i = 0;

		for (BMBakedFile &bmbf : bakedFiles) {

			BakedFile &out = file.files[i];
			out.inputs.resize(bmbf.inputs);
			out.outputs.resize(bmbf.outputs);
			out.dependencies.resize(bmbf.dependencies);

			for (u32 j = 0; j < bmbf.inputs; ++j)
				out.inputs[j] = file.strings.names[bakedInfop[j]];

			bakedInfop += bmbf.inputs;

			for (u32 j = 0; j < bmbf.outputs; ++j)
				out.outputs[j] = file.strings.names[bakedInfop[j]];

			bakedInfop += bmbf.outputs;

			for (u32 j = 0; j < bmbf.dependencies; ++j)
				out.dependencies[j] = file.strings.names[bakedInfop[j]];

			bakedInfop += bmbf.dependencies;
			++i;
		}

		buf.deconstruct();

	}
	else Log::println("No cached BakingManager found; rebaking everything...");

}

int BakeManager::run() {

	Log::println("BakingManager started...");

	for (BakeOption &bo : bakeOptions) {

		std::unordered_map<String, std::vector<String>> paths;
		std::unordered_map<String, String> types;

		FileManager::get()->foreachFileRecurse(bo.path, [&](const FileInfo &fi) -> bool {

			if (!fi.isFolder) {

				for (auto &elem : bo.inputExtensions) {

					String origin = "";

					for (const String &elem0 : elem.second)
						if (fi.name.endsWithIgnoreCase(String(".") + elem0)) {
							origin = fi.name.cutEnd(fi.name.size() - elem0.size() - 1);

							auto it = types.find(origin);

							if (it == types.end())
								types[origin] = elem.first;
							else if (it->second != elem.first) {
								Log::println(String("There was already an entry named \"") + origin.replaceFirst("mod/", "res/") + "\" but the types didn't match (" + elem.first + " vs " + types[origin] + ")");
								return false;
							}

							break;
						}

					if (origin != "") {
						paths[origin].push_back(fi.name);
						return false;
					}

				}
			}

			return false;

		});

		for (auto &elem : paths) {

			BakedFile bf;
			bf.file = elem.first;
			bf.inputs = elem.second;
			bf.extension = types[elem.first];

			if (!shouldUpdate(bf)) {
				Log::println(bf.file.replaceFirst("mod/", "res/") + " is already updated");
				continue;
			}

			bo.bake(bf, stripDebug);
			cache(bf);
			Log::println(bf.file.replaceFirst("mod/", "res/") + " has been updated");

		}

	}

	write();
	Log::println("Baking finished");
	return 0;

}

bool BakeManager::shouldUpdate(BakedFile &bf) {

	BakedFile *which = nullptr;

	for (BakedFile &bf0 : file.files)
		if (bf0.inputs == bf.inputs) {
			which = &bf0;
			break;
		}

	if (which == nullptr)
		return true;

	for (String &out : which->outputs) {

		if (!FileManager::get()->fileExists(out))
			return true;

		FileInfo oinfo = FileManager::get()->getFile(out);

		for (String &in : which->inputs) {

			FileInfo iinfo = FileManager::get()->getFile(in);

			if (iinfo.modificationTime > oinfo.modificationTime)
				return true;

		}

		for (String &in : which->dependencies) {

			FileInfo dinfo = FileManager::get()->getFile(in);

			if (dinfo.modificationTime > oinfo.modificationTime)
				return true;

		}

	}

	return false;
}

void BakeManager::cache(BakedFile &bf) {

	changed = true;

	for (BakedFile &bf0 : file.files)
		if (bf0.inputs == bf.inputs) {
			bf0 = bf;
			return;
		}

	file.files.push_back(bf);

}

u16 BakeManager::makeFlags() {
	return stripDebug ? BMFlag::StripDebug.value : 0;
}

void BakeManager::write() {

	if (changed) {

		u32 size = u32(sizeof(BMHeader) + file.files.size() * sizeof(BMBakedFile));

		memcpy(file.header.header, "oiBM", 4);
		file.header.files = (u16)file.files.size();
		file.header.version = BMHeader::getGlobalVersion();
		file.header.flags = makeFlags();

		file.strings.keyset = "";
		file.strings.names.clear();

		for (BakedFile &bmbf : file.files) {

			size += u32(bmbf.inputs.size() + bmbf.outputs.size() + bmbf.dependencies.size()) * 2;

			std::vector<String> strings = bmbf.inputs;
			strings.insert(strings.end(), bmbf.outputs.begin(), bmbf.outputs.end());
			strings.insert(strings.end(), bmbf.dependencies.begin(), bmbf.dependencies.end());

			for (String &str : strings) {

				file.strings.add(str);

				for (char c : str)
					if (!file.strings.keyset.contains(c))
						file.strings.keyset += c;

			}

		}

		Buffer buf = oiSL::write(file.strings);

		Buffer output(size + buf.size());
		output.clear();
		output.copy(buf, buf.size(), 0, size);
		buf.deconstruct();

		Buffer write = output;
		write.template operator[]<BMHeader>(0) = file.header;
		write += u32(sizeof(BMHeader));

		u32 siz = u32(file.files.size() * sizeof(BMBakedFile));

		u16 *ptr = (u16*)(write.addr() + siz);
		BMBakedFile *bptr = (BMBakedFile*)write.addr();

		for (BakedFile &bmbf : file.files) {

			std::vector<String> strings = bmbf.inputs;
			strings.insert(strings.end(), bmbf.outputs.begin(), bmbf.outputs.end());
			strings.insert(strings.end(), bmbf.dependencies.begin(), bmbf.dependencies.end());

			for (String &str : strings) {
				*ptr = (u16)file.strings.lookup(str);
				++ptr;
			}

			bptr->inputs = (u8)bmbf.inputs.size();
			bptr->outputs = (u8)bmbf.outputs.size();
			bptr->dependencies = (u16)bmbf.dependencies.size();
			++bptr;

		}

		if (!FileManager::get()->write(location, output))
			Log::error(String("Couldn't write cache to \"") + location.replaceFirst("mod/", "res/") + "\"");
		else {
			Log::println(String("Wrote cache to \"") + location.replaceFirst("mod/", "res/") + "\"");
			changed = false;
		}

		output.deconstruct();
	}

}