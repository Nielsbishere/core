#include "Graphics/ResourceManager.h"
#include "Graphics/GPU/GraphicsResource.h"
using namespace oi::gc;
using namespace oi;

void ResourceManager::initGPU() {

	for (auto &resource : resources)
		initGPU(resource.first);
}


void ResourceManager::initGPU(OString name) {

	auto resource = resources.find(name);

	if (resource == resources.end()) return;

	if (resource->second.state < ResourceState::GPU)
		if (resource->second.gr->init())
			resource->second.state = ResourceState::GPU;
}

OString ResourceManager::getPath(OString handle) {

	if (handle.size() < 3) return "";

	OString type = handle.cutEnd(3);

	auto it = resourceTypes.find(type);
	if (it == resourceTypes.end()) return "";

	return OString("resources/") + it->second + "/" + handle.cutBegin(4);
}

void ResourceManager::initCPU() {
	for (auto &resource : resources)
		initCPU(resource.first);
}

void ResourceManager::initCPU(OString name) {

	auto resource = resources.find(name);

	if (resource == resources.end()) return;

	if (resource->second.state < ResourceState::CPU)
		if (resource->second.gr->initData(getPath(resource->first)))
			resource->second.state = ResourceState::CPU;
}