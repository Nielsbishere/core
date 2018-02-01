#include "Graphics/GraphicsInterface.h"
#include <Input/InputHandler.h>
using namespace oi::gc;
using namespace oi;

GraphicInterface::GraphicInterface() : gl(Graphics::get()) { }

void GraphicInterface::init() {
	gl->init(getParent());
	initScene();

	getInput().load("Resources/Settings/Input.json");
}

void GraphicInterface::render() {
	gl->clear(RGBAf(1, 1, 0, 1));
	renderScene();
	getParent()->swapBuffers();

	//Vec3 mov = getInput().getAxis("Move");
}

void GraphicInterface::initScene() {
	s = gl->compileShader(ShaderInfo("Resources/Shaders/test", ShaderType::NORMAL));

	struct Vertex {
		Vec3 pos;
	};

}

void GraphicInterface::renderScene() {
	s->bind();

	s->unbind();
}

GraphicInterface::~GraphicInterface() {
	delete s;
	delete gl;
}