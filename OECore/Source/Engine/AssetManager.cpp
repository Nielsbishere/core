#include "Engine/AssetManager.h"
#include <Graphics/GPU/GraphicsResource.h>
#include <Types/Thread.h>
#include <algorithm>
#include <Graphics/Material/ShaderInfo.h>
#include <Graphics/Material/TextureInfo.h>
#include <Graphics/Material/SamplerInfo.h>

using namespace oi::ec;
using namespace oi::gc;
using namespace oi;

AssetManager::AssetManager() : assetIndex(0) {}
AssetManager::~AssetManager() {
	unload();
}

void AssetManager::unloadGPU() {
	for (auto &a : assets) {
		if (a.second.state.getValue() == AssetState::GpuLoaded) {
			a.second.state = AssetState::CpuLoaded;
			a.second.getResource()->destroy();
		}
	}
}

void AssetManager::unloadCPU() {
	for (auto &a : assets) {
		if (a.second.state.getValue() == AssetState::CpuLoaded) {
			a.second.state = AssetState::NotLoaded;
			a.second.getResource()->destroyData();
		}
	}
}

void AssetManager::unload() {

	for (auto &a : assets)
		delete a.second.getResource();

	assets.clear();
	assetIndex = 0;
}

u32 AssetManager::size() { return (u32) assets.size(); }

AssetInfo AssetManager::find(u32 i) {
	std::unordered_map<u32, AssetInfo>::iterator it;
	if ((it = assets.find(i)) == assets.end()) return AssetInfo();
	return it->second;
}

AssetInfo AssetManager::find(OString name) {
	auto it = std::find_if(assets.begin(), assets.end(), [name](const std::unordered_map<u32, AssetInfo>::value_type &v) -> bool { return v.second.getName() == name; });
	if (it == assets.end()) return AssetInfo();
	return it->second;
}

AssetInfo AssetManager::findByPath(OString path) {
	auto it = std::find_if(assets.begin(), assets.end(), [path](const std::unordered_map<u32, AssetInfo>::value_type &v) -> bool { return v.second.getPath() == path; });
	if (it == assets.end()) return AssetInfo();
	return it->second;
}

u32 AssetManager::create(AssetInfo info) {
	info.assetHandle = assetIndex;
	assets[assetIndex] = info;
	++assetIndex;
	return info.assetHandle;
}

bool AssetManager::initData(u32 index) {

	AssetInfo info = find(index);

	if (info.isNull()) return false;

	if (info.state.getValue() >= AssetState::CpuLoaded) return true;

	bool result = info.getResource()->initData(info.getPath());
	
	if (result)
		info.state = AssetState::CpuLoaded;

	return true;
}

bool AssetManager::initResource(JSON &json, OString path, u32 index, OString type) {

	if (type.startsWithIgnoreCase("Shader"))
		return initCPU(index, ShaderInfo(path, json.getString(path + "/type", "Normal")));

	else if (type.startsWithIgnoreCase("Texture")) {

		if (json.getString(path + "/path") == "-")		//Empty texture

			if (json.exists(path + "/length"))
				return initCPU(index, TextureInfo(json.getUInt(path + "/width", 32), json.getUInt(path + "/height", 32), json.getUInt(path + "/length", 32), TextureLayout(json.getString(path + "/type", "RGBAc"))));

			else if (json.exists(path + "/height"))
				return initCPU(index, TextureInfo(json.getUInt(path + "/width", 512), json.getUInt(path + "/height", 512), TextureLayout(json.getString(path + "/type", "RGBAc"))));

			else if (json.exists(path + "/width"))
				return initCPU(index, TextureInfo(json.getUInt(path + "/width", 1024), TextureLayout(json.getString(path + "/format", "RGBAc"))));

			else
				return Log::error("Empty texture requires width/height/length and format specifier");

		else 
			return initCPU(index, TextureInfo(json.getString(path + "/type", "Texture2D"), TextureLayout(json.getString(path + "/format", "RGBAc"))));
	}

	else if (type.startsWithIgnoreCase("Sampler")) {

		if (json.exists(path + "/Wrapping/2"))
			return initCPU(index, SamplerInfo(json.getString(path + "/Wrapping/0", "Repeat"), json.getString(path + "/Wrapping/1", "Repeat"), json.getString(path + "/Wrapping/2", "Repeat"), json.getString(path + "/Filtering/0", "LinearMip"), json.getString(path + "/Filtering/1", "Linear")));

		else if (json.exists(path + "/Wrapping/1"))
			return initCPU(index, SamplerInfo(json.getString(path + "/Wrapping/0", "Repeat"), json.getString(path + "/Wrapping/1", "Repeat"), json.getString(path + "/Filtering/0", "LinearMip"), json.getString(path + "/Filtering/1", "Linear")));

		else if (json.exists(path + "/Wrapping/0"))
			return initCPU(index, SamplerInfo(json.getString(path + "/Wrapping/0", "Repeat"), json.getString(path + "/Filtering/0", "LinearMip"), json.getString(path + "/Filtering/1", "Linear")));

		else
			return initCPU(index, SamplerInfo(SamplerWrapping::Repeat, json.getString(path + "/Filtering/0", "LinearMip"), json.getString(path + "/Filtering/1", "Linear")));

	}
	else if (type.startsWithIgnoreCase("Setting")) {
		//Load into InputManager
	}
	
	return Log::error(OString("Couldn't interpret resource \"") + path + "\" from Scene json");
}

u32 AssetManager::addResources(JSON &json, OString path) {

	u32 startIndex = assetIndex;

	std::vector<std::pair<OString, OString>> types;

	for(OString type : json.getMemberIds(path))
		for (OString resource : json.getMemberIds(path + "/" + type)) {
			AssetInfo info = AssetInfo(resource, OString("Resources/") + type + "/" + json.getString(path + "/" + type + "/" + resource + "/path"));

			if (info.getName() != "") {
				create(info);
				types.push_back({ type, resource });
			}
		}

	u32 count = size() - startIndex;

	if (count / Thread::cores() < 2) {

		u32 initialized = 0;

		for (u32 i = startIndex; i < startIndex + count; ++i) {
			u32 j = i - startIndex;
			const OString &type = types[j].first;
			const OString &resource = types[j].second;

			if (initResource(json, path + "/" + type + "/" + resource, i, type))
				++initialized;
		}

		return initialized;
	}

	u32 remainder = count % Thread::cores();
	u32 division = count / Thread::cores();

	auto vec = Thread::foreachCore<u32>([&](u32 i) -> u32 {
		
		u32 num = i == 0 ? remainder + division : division;
		u32 start = startIndex + division * i + (i > 0 ? remainder : 0);

		u32 initialized = 0;

		for (u32 j = start; j < start + num; ++j) {
			u32 k = j - startIndex;
			const OString &type = types[k].first;
			const OString &resource = types[k].second;

			if (initResource(json, path + "/" + type + "/" + resource, j, type))
				++initialized;
		}

		return initialized;
	});

	u32 init = 0;

	for (u32 i : vec)
		init += i;

	return init;
}

u32 AssetManager::addResources(OString path) {
	return addResources((JSON) OString::readFromFile(path), "Resources");
}

void AssetManager::initGPU() {
	for (auto &a : assets)
		if (a.second.state.getValue() == AssetState::CpuLoaded)
			if (a.second.getResource()->init())
				a.second.state = AssetState::GpuLoaded;
}

void AssetManager::init(OString path) {
	addResources(path);
	initGPU();
}

gc::GraphicsResource *AssetManager::operator[](u32 i) {
	AssetInfo info = find(i);
	if (info.isNull()) return nullptr;
	return info.getResource();
}