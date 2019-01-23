#include "input/inputdevice.h"
using namespace oi::wc;
using namespace oi;

InputDevice::InputDevice(InputType type, u32 id) : type(type), id(id) {}
InputDevice::~InputDevice() {}

bool InputDevice::isDown(Binding b) const { return getState(b) == InputState::DOWN; }
bool InputDevice::isUp(Binding b) const { return getState(b) == InputState::UP; }
bool InputDevice::isPressed(Binding b) const { return getState(b) == InputState::PRESSED; }
bool InputDevice::isReleased(Binding b) const { return getState(b) == InputState::RELEASED; }

u32 InputDevice::getId() const { return id; }
const InputType InputDevice::getType() const { return type; }

f32 InputDevice::toAxis(InputState state, bool delta) const {

	if (delta)
		return state == InputState::RELEASED ? -1.f : (state == InputState::PRESSED ? 1.f : 0.f);

	return state == InputState::DOWN ? 1.f : 0.f; 
}