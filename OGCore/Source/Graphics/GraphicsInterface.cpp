#include "Graphics/GraphicsInterface.h"
#include "Graphics/GraphicsManager.h"
#include <Window/Window.h>
using namespace oi::gc;
using namespace oi;

GraphicsInterface::GraphicsInterface(GraphicsManager &gm) : gl(gm.gl) { }

void GraphicsInterface::init() {
	gl->init(getParent());
	initScene();
}

void GraphicsInterface::render() {
	gl->viewport(getParent());
	gl->clear();
	renderScene();
	getParent()->swapBuffers();
}