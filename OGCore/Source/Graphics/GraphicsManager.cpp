#include "Graphics/GraphicsManager.h"
#include <Utils/Log.h>
#include <Utils/DLLLoader.h>
using namespace oi::gc;
using namespace oi;

Graphics *GraphicsManager::load(OString path) {

	DLLLoader loader(path);
	
	if (!loader.isValid()) {
		Log::throwError<Graphics, 0x1>(OString("Graphics load(OString) couldn't find the requested dll (\"") + path + "\")");
		return nullptr;
	}

	auto intialize = loader.getFunc<f_createGraphics>("createGraphics");

	if (intialize == nullptr) {
		Log::throwError<Graphics, 0x2>("Graphics load(OString) couldn't find requested function in dll");
		return nullptr;
	}

	current = path;
	return gl = intialize(this);
}

void GraphicsManager::switchGraphics(OString path) {
	if (path != current) {
		endGraphics();
		startGraphics(path);
	}
}

OString getGraphics(GraphicLibrary gl) {
	switch (gl) {
	case GraphicLibrary::DirectX:
		return "OGDX";
	case GraphicLibrary::Vulkan:
		return "OGV";
	case GraphicLibrary::MetalGL:
		return "OGMGL";
	default:
		return "OGOGL";
	}
}

void GraphicsManager::switchGraphics(GraphicLibrary gl) {

	if(gl > GraphicLibrary::MetalGL)
		Log::throwError<Graphics, 0x0>("Graphics switchGraphics doesn't work for unknown GLs, please use switchGraphics(OString) instead");
	else
		switchGraphics(getGraphics(gl));

}

void GraphicsManager::endGraphics() {

	if (gl != nullptr)
		delete gl;

	gl = nullptr;
	current = "";
}

void GraphicsManager::startGraphics(OString path) {

	if (current == "") {

		if (path == "")
			path = getGraphics(graphicLibrary);

		load(path);
	}

}

GraphicsManager::GraphicsManager() {
	startGraphics();
}

GraphicsManager::~GraphicsManager() {
	endGraphics();
}