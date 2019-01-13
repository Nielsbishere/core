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

	InputDeviceBinding type = InputDeviceBinding((u32) InputDeviceBinding::MISC + b.getControllerId());

	switch (b.getInputType()) {

	case InputType::KEYBOARD:
		type = InputDeviceBinding::KEYBOARD;
		break;

	case InputType::MOUSE:
		type = InputDeviceBinding::MOUSE;
		break;

	case InputType::CONTROLLER:
		type = (InputDeviceBinding)((u32)InputDeviceBinding::CONTROLLER0 + b.getControllerId());
		break;

	}

	return getDevice(type);

}

Keyboard *InputHandler::getKeyboard() { return dynamic_cast<Keyboard*>(getDevice(InputDeviceBinding::KEYBOARD)); }
Mouse *InputHandler::getMouse() { return dynamic_cast<Mouse*>(getDevice(InputDeviceBinding::MOUSE)); }

Controller *InputHandler::getController(u32 i) {

	if (i >= getControllers())
		return nullptr;

	return dynamic_cast<Controller*>(getDevice((InputDeviceBinding)((u32)InputDeviceBinding::CONTROLLER0 + i))); 
}

void InputHandler::update(Window *w, f32 dt) {
	for (auto dev : devices)
		if(dev.second != nullptr)
			dev.second->update(w, dt);
}