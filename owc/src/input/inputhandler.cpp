#include "input/inputhandler.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/controller.h"
using namespace oi;
using namespace wc;

InputDevice *InputHandler::getDevice(InputDeviceBinding binding) {

	auto it = devices.find(binding);
	
	if (it == devices.end()) 
		return nullptr;

	return it->second;
}

InputDevice *InputHandler::getDevice(Binding b) {

	switch (b.getInputType()) {

	case InputType::KEYBOARD:
		return getDevice(InputDeviceBinding::KEYBOARD);

	case InputType::MOUSE:
		return getDevice(InputDeviceBinding::MOUSE);

	case InputType::CONTROLLER:
		return getDevice((InputDeviceBinding)((u32)InputDeviceBinding::CONTROLLER0 + b.getControllerId()));

	default:
		return getDevice((InputDeviceBinding)(b.getControllerId() + (u32) InputDeviceBinding::MISC));
	}

}

Keyboard *InputHandler::getKeyboard() { return dynamic_cast<Keyboard*>(getDevice(InputDeviceBinding::KEYBOARD)); }
Mouse *InputHandler::getMouse() { return dynamic_cast<Mouse*>(getDevice(InputDeviceBinding::MOUSE)); }

Controller *InputHandler::getController(u32 i) {

	if (i >= (u32) InputDeviceBinding::CONTROLLERS)
		return nullptr;

	return dynamic_cast<Controller*>(getDevice((InputDeviceBinding)((u32)InputDeviceBinding::CONTROLLER0 + i))); 
}

void InputHandler::update(Window *w, flp dt) {
	for (auto dev : devices)
		dev.second->update(w, dt);
}