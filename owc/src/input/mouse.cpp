#include "input/mouse.h"
using namespace oi::wc;
using namespace oi;

Mouse::Mouse() : InputDevice(InputType::MOUSE) {}

InputState Mouse::getState(Binding b) const {

	if (b.toMouseButton() == MouseButton::Undefined)
		return InputState::UP;

	bool p = prev[b.getCode() - 1], n = next[b.getCode() - 1];

	if (!p && !n) return InputState::UP;
	if (p && n) return InputState::DOWN;
	if (!p && n) return InputState::PRESSED;
	return InputState::RELEASED;
}

f32 Mouse::getAxis(Binding b) const {

	if (b.getBindingType() != BindingType::MOUSE_AXIS)
		return toAxis(getState(b));

	if (b.getCode() == 0) return 0.f;

	return axes[b.getCode() - 1];
}

void Mouse::update(Binding b, bool down) {
	if (b.getBindingType() != BindingType::MOUSE_BUTTON || b.getCode() == 0) return;

	next[b.getCode() - 1] = down;
}