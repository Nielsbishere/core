#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "input/inputhandler.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/controller.h"
using namespace oi::wc;

u32 InputHandler::getControllers(){
	return 1U;
}

void InputHandler::init() {

	devices[InputDeviceBinding::KEYBOARD] = new Keyboard();
	devices[InputDeviceBinding::MOUSE] = new Mouse();

	for (u32 i = 0; i < getControllers(); ++i)
		devices[(InputDeviceBinding)((u32) InputDeviceBinding::CONTROLLER0 + i)] = new Controller(i);

}

#endif