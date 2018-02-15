#include "Graphics/GraphicsInterface.h"
#include "Graphics/Graphics.h"
#include <Window/Window.h>
using namespace oi::gc;
using namespace oi;

GraphicsInterface::GraphicsInterface() : gl(Graphics::get()) { }

void GraphicsInterface::init() {
	gl->init(getParent());
	initScene();
}

void GraphicsInterface::render() {
	gl->viewport(getParent());
	gl->clear(Vec3(1, 0, 1));
	renderScene();
	getParent()->swapBuffers();
}