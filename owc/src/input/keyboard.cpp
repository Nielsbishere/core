#include "input/keyboard.h"
using namespace oi::wc;
using namespace oi;

Keyboard::Keyboard() : InputDevice(InputType::KEYBOARD) {}

InputState Keyboard::getState(Binding b) const {

	if (b.toKey() == Key::Undefined)
		return InputState::UP;

	bool p = prev[b.getCode() - 1], n = next[b.getCode() - 1];

	if (!p && !n) return InputState::UP;
	if (p && n) return InputState::DOWN;
	if (!p && n) return InputState::PRESSED;
	return InputState::RELEASED;
}

f32 Keyboard::getAxis(Binding b) const {

	if (b.toKey() == Key::Undefined)
		return 0.f;

	return toAxis(getState(b));
}

void Keyboard::update(Binding b, bool down) {
	if (b.getBindingType() != BindingType::KEYBOARD || b.getCode() == 0) return;
	next[b.getCode() - 1] = down;
}