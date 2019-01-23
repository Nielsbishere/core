#ifdef __ANDROID__

#include "input/controller.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;
using namespace oi;

Controller::~Controller() {}

void Controller::update(Window *w, f32 dt) {

	if (axes[prevAxes] != u16_MAX)
		for (u32 i = 0; i < axisCount; ++i)
			axes[deltaAxes + i] = axes[i] - axes[prevAxes + i];

	prev = next;
	memcpy(axes + prevAxes, axes, axesSize);

}

void Controller::vibrate(Vec2 amount, f32 time) {
	Log::warn("Controller::vibrate isn't supported on Android");
}

#endif