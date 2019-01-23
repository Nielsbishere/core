#include "input/mouse.h"
using namespace oi::wc;
using namespace oi;

Mouse::Mouse() : InputDevice(InputType::MOUSE) { memset(axes, 0, sizeof(axes)); axes[prevAxes] = u16_MAX; }

InputState Mouse::getState(Binding b) const {

	if (b.toMouseButton() == MouseButton::Undefined)
		return InputState::UP;

	bool p = prev[b.getCode() - 1], n = next[b.getCode() - 1];

	if (!p && !n) return InputState::UP;
	if (p && n) return InputState::DOWN;
	if (!p && n) return InputState::PRESSED;
	return InputState::RELEASED;
}

f32 Mouse::getAxis(Binding b, bool delta) const {

	if (b.getBindingType() != BindingType::MOUSE_AXIS)
		return toAxis(getState(b), delta);

	if (b.getCode() == 0) return 0.f;

	return axes[b.getCode() - 1 + delta * deltaAxes];
}

void Mouse::update(Binding b, bool down) {

	if (b.getBindingType() != BindingType::MOUSE_BUTTON || b.getCode() == 0) return;

	next[b.getCode() - 1] = down;
}

void Mouse::update(Window*, f32) {

	if (axes[prevAxes] != u16_MAX)
		for (u32 i = 0; i < axisCount; ++i)
			axes[deltaAxes + i] = axes[i] - axes[prevAxes + i];

	prev = next;
	memcpy(axes + prevAxes, axes, axesSize);
}