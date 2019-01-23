#include "input/controller.h"
using namespace oi::wc;
using namespace oi;

Controller::Controller(u32 id) : InputDevice(InputType::CONTROLLER, id) { memset(axes, 0, sizeof(axes)); axes[prevAxes] = u16_MAX; }

InputState Controller::getState(Binding b) const {

	if (b.toButton() == ControllerButton::Undefined)
		return InputState::UP;

	bool p = prev[b.getCode() - 1], n = next[b.getCode() - 1];

	if (!p && !n) return InputState::UP;
	if (p && n) return InputState::DOWN;
	if (!p && n) return InputState::PRESSED;
	return InputState::RELEASED;
}

f32 Controller::getAxis(Binding b, bool delta) const {

	if (b.getBindingType() != BindingType::CONTROLLER_AXIS)
		return toAxis(getState(b), delta);

	if (b.getCode() == 0) return 0.f;

	return axes[b.getCode() - 1 + deltaAxes * delta];
}

void Controller::update(Binding b, bool down) {
	
	if (b.getBindingType() != BindingType::CONTROLLER_BUTTON || b.getCode() == 0 || b.getControllerId() != getId()) return;

	next[b.getCode() - 1] = down;
}