#include "graphics/graphicsinterface.h"
#include <window/window.h>
using namespace oi::gc;
using namespace oi;

void GraphicsInterface::init() {
	g.init(getParent());
	initScene();
}

void GraphicsInterface::initSurface() {
	g.initSurface(getParent());
}

void GraphicsInterface::destroySurface() {
	g.destroySurface();
}

void GraphicsInterface::render() {
	g.begin();
	renderScene();
	g.end();
}

void GraphicsInterface::onResize(Vec2u res){
	destroySurface();
	initSurface();
}