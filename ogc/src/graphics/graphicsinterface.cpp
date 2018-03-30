#include <graphics/graphicsinterface.h>
using namespace oi::gc;

void GraphicsInterface::init() {
	g.init(getParent());
	initScene();
}

void GraphicsInterface::render() {
	g.clear(Vec4f(0.25f, 0.5f, 1.0f, 1.0f));
	renderScene();
}