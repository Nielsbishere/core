#include "input/inputdevice.h"
#include <algorithm>
using namespace oi::wc;
using namespace oi;

InputDevice::InputDevice(InputType type, u32 id) : type(type), id(id) {}
InputDevice::~InputDevice() {}

bool InputDevice::isDown(Binding b) const { return getState(b) == InputState::DOWN; }
bool InputDevice::isUp(Binding b) const { return getState(b) == InputState::UP; }
bool InputDevice::isPressed(Binding b) const { return getState(b) == InputState::PRESSED; }
bool InputDevice::isReleased(Binding b) const { return getState(b) == InputState::RELEASED; }

flp InputDevice::toAxis(InputState state) const { return (flp)(state == InputState::DOWN ? 1 : 0); }

u32 InputDevice::getId() const { return id; }
const InputType InputDevice::getType() const { return type; }