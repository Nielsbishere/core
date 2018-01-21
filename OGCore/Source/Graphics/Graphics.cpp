#include "Graphics/Graphics.h"
#include <Utils/Log.h>
#include <Utils/DLLLoader.h>
using namespace oi::gc;
using namespace oi;

Graphics *Graphics::instance = nullptr;

Graphics *Graphics::load(OString path) {

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

	return instance = intialize();
}

void Graphics::switchGraphics(OString path) {
	endGraphics();
	startGraphics(path);
}

void Graphics::switchGraphics(GraphicLibrary gl) {

	if (graphicLibrary == GraphicLibrary::OpenGL)
		return switchGraphics("OGOGL.dll");

	if (graphicLibrary == GraphicLibrary::DirectX)
		return switchGraphics("OGDX.dll");

	if (graphicLibrary == GraphicLibrary::Vulkan)
		return switchGraphics("OGV.dll");

	if (graphicLibrary == GraphicLibrary::MetalGL)
		return switchGraphics("OGMGL.dll");

	Log::throwError<Graphics, 0x0>("Graphics switchGraphics doesn't work for unknown GLs, please use switchGraphics(OString) instead");
}

void Graphics::endGraphics() {
	if(instance != nullptr)
		delete instance;
	instance = nullptr;
}

void Graphics::startGraphics(OString path) {

	if (path == "") {

		if (graphicLibrary == GraphicLibrary::OpenGL)
			load("OGOGL.dll");

		if (graphicLibrary == GraphicLibrary::DirectX)
			load("OGDX.dll");

		if (graphicLibrary == GraphicLibrary::Vulkan)
			load("OGV.dll");

		if (graphicLibrary == GraphicLibrary::MetalGL)
			load("OGMGL.dll");

		return;
	}

	load(path);
}

Graphics *&Graphics::get() { return instance; }