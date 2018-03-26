#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "input/controller.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;
using namespace oi;

Controller::~Controller() {}

void Controller::update(Window *w, flp dt) {

	WindowInterface *wi = w->getInterface();

	if (wi != nullptr){

		if (prevConnected && !connected)
			wi->onDisconnect(this);
		else if (!prevConnected && connected)
			wi->onConnect(this);

		for (u32 i = 1U; i < ControllerButton::length; ++i) {

			Binding b = Binding(ControllerButton(i), getId());
			InputState state = getState(b);

			if (state == InputState::PRESSED)
				wi->onInput(this, b, true);
			else if (state == InputState::RELEASED)
				wi->onInput(this, b, false);
		}
	}

	prev = next;
	prevConnected = connected;
}

void Controller::vibrate(Vec2 amount, f32 time) {
	Log::warn("Controller::vibrate isn't supported on Android");
}

#endif