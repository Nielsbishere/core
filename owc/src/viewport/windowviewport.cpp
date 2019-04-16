#include "viewport/windowviewport.hpp"
#include "window/windowinterface.h"
using namespace oi::wc;
using namespace oi;

void WindowViewport::setInterface(WindowInterface *wif) {

	wi = wif;
	finalizeCount = 0;

	if (!wi)
		return;

	wi->init();
	wi->initSurface(getLayer(0).size);
	wi->onAspectChange(Vec2(getLayer(0).size).getAspect());
}

f32 WindowViewport::updateInternal() {

	if (!wi || getLayer(0).size.x == 0 || getLayer(0).size.y == 0) {
		lastTick = Timer::getGlobalTimer().getDuration();
		return 0;
	}

	f32 dt = Timer::getGlobalTimer().getDuration() - lastTick;
	wi->update(dt);
	lastTick = Timer::getGlobalTimer().getDuration();
	wi->render();
	return dt;
}