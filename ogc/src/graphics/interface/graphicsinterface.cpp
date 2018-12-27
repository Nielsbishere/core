#include "window/window.h"
#include "graphics/interface/graphicsinterface.h"
#include "graphics/objects/render/commandlist.h"
using namespace oi::gc;
using namespace oi;

void GraphicsInterface::init() {
	g.init(getParent());
}

void GraphicsInterface::initSurface(Vec2u res) {

	g.initSurface(getParent());

	if (!initialized) {
		initScene();
		initialized = true;
	}

	initSceneSurface(res);

}

void GraphicsInterface::destroySurface() {
	g.finish();
	destroySceneSurface();
	g.destroySurface();
}

void GraphicsInterface::render() {
	g.begin();
	renderScene();
	g.end();
}

void GraphicsInterface::onResize(Vec2u res){
	destroySurface();
	initSurface(res);
}