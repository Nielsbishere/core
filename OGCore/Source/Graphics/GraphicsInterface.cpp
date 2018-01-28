#include "Graphics/GraphicsInterface.h"
#include <Input/InputHandler.h>
using namespace oi::gc;
using namespace oi;

GraphicInterface::GraphicInterface() : gl(Graphics::get()) { }

void GraphicInterface::init() {
	gl->init(getParent());
	initScene();

	getInput().bind("Shoot", OString("Cross button #0"));
	getInput().bind("Shoot", OString("R1 button #0"));
	getInput().bind("Shoot", OString("L1 button #0"));
	getInput().bind("Shoot", OString("A key"));
	getInput().bind("Woof", OString("Backspace key"));
	getInput().bind("Shoot", OString("Left mouse"));

	getInput().bindAxis("Move", wc::InputAxis(OString("D key"), wc::InputAxis1D::X, 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("A key"), wc::InputAxis1D::X, -1));
	getInput().bindAxis("Move", wc::InputAxis(OString("E key"), wc::InputAxis1D::Y, 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("Q key"), wc::InputAxis1D::Y, -1));
	getInput().bindAxis("Move", wc::InputAxis(OString("W key"), wc::InputAxis1D::Z, 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("S key"), wc::InputAxis1D::Z, -1));

	getInput().bindAxis("Move", wc::InputAxis(OString("Lx axis #0"), wc::InputAxis1D::X, 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("L2 axis #0"), wc::InputAxis1D::Y, 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("R2 axis #0"), wc::InputAxis1D::Y,- 1));
	getInput().bindAxis("Move", wc::InputAxis(OString("Ly axis #0"), wc::InputAxis1D::Z, 1));

	Log::println(getInput().write());
}

void GraphicInterface::render() {
	gl->clear(RGBAf(1, 1, 0, 1));
	renderScene();
	getParent()->swapBuffers();

	/*Vec3 mov = getInput().getAxis("Move");
	printf("%f %f %f\n", mov.x(), mov.y(), mov.z());*/
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