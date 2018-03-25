#include <template/platformdefines.h>

#ifdef __WINDOWS__

#include "input/inputhandler.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/controller.h"
using namespace oi::wc;

void InputHandler::init() {

	devices[InputDeviceBinding::KEYBOARD] = new Keyboard();
	devices[InputDeviceBinding::MOUSE] = new Mouse();

	for (u32 i = 0; i < (u32)InputDeviceBinding::CONTROLLERS; ++i)
		devices[(InputDeviceBinding)((u32) InputDeviceBinding::CONTROLLER0 + i)] = new Controller(i);

}

#endif