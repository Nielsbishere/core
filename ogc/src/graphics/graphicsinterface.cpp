#include <graphics/graphicsinterface.h>
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
	g.clear(Vec4f(0.25f, 0.5f, 1.0f, 1.0f));
	renderScene();
	g.swapBuffers();
}

void GraphicsInterface::onResize(Vec2u res){
	destroySurface();
	initSurface();
}