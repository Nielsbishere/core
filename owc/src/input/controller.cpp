#include "input/controller.h"
using namespace oi::wc;
using namespace oi;

Controller::Controller(u32 id) : InputDevice(InputType::CONTROLLER, id) {}

InputState Controller::getState(Binding b) const {

	if (b.toButton() == ControllerButton::Undefined)
		return InputState::UP;

	bool p = prev[b.getCode() - 1], n = next[b.getCode() - 1];

	if (!p && !n) return InputState::UP;
	if (p && n) return InputState::DOWN;
	if (!p && n) return InputState::PRESSED;
	return InputState::RELEASED;
}

f32 Controller::getAxis(Binding b) const {

	if (b.getBindingType() != BindingType::CONTROLLER_AXIS)
		return toAxis(getState(b));

	if (b.getCode() == 0) return 0.f;

	return axes[b.getCode() - 1];
}

void Controller::update(Binding b, bool down) {
	
	if (b.getBindingType() != BindingType::CONTROLLER_BUTTON || b.getCode() == 0 || b.getControllerId() != getId()) return;

	next[b.getCode() - 1] = down;
}