#ifdef __ANDROID__

#include "input/controller.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;
using namespace oi;

Controller::~Controller() {}

void Controller::update(Window *w, flp dt) {
	prev = next;
}

void Controller::vibrate(Vec2 amount, f32 time) {
	Log::warn("Controller::vibrate isn't supported on Android");
}

#endif