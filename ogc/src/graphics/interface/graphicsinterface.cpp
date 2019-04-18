#include "window/window.h"
#include "graphics/interface/graphicsinterface.h"
#include "graphics/objects/render/commandlist.h"
using namespace oi::gc;
using namespace oi;

void GraphicsInterface::init() {
	g.init(getParent());
}

void GraphicsInterface::initSurface(Vec2u res) {

	if(res != Vec2u())
		g.initSurface(getParent());

	if (!initialized) {
		initScene();
		initialized = true;
	}

	initSceneSurface(res);

}

void GraphicsInterface::destroySurface() {
	g.finish();
	initSurface({});
	g.destroySurface();
}

void GraphicsInterface::render() {
	g.begin();
	renderScene();
	g.end();
}

void GraphicsInterface::onResize(Vec2u res){
	g.finish();
	g.destroySurface();
	initSurface(res);
}