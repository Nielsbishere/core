#include <window/window.h>
#include "graphics/graphicsinterface.h"
#include "graphics/commandlist.h"
using namespace oi::gc;
using namespace oi;

void GraphicsInterface::init() {

	g.init(getParent());

	cmdList = g.create("Default command list", CommandListInfo());
	g.use(cmdList);

}

void GraphicsInterface::initSurface() {

	g.initSurface(getParent());

	if (!initialized) {
		initScene();
		initialized = true;
	}

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