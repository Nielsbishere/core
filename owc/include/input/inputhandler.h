#pragma once
#include "inputdevice.h"

namespace oi {

	namespace wc {

		class Keyboard;
		class Mouse;
		class Controller;

		enum class InputDeviceBinding {
			KEYBOARD,
			MOUSE,
			CONTROLLER0, CONTROLLER1, CONTROLLER2, CONTROLLER3,
			MISC,

			CONTROLLERS = 4
		};

		class InputHandler {

		public:

			void init();
			void update(Window *w, flp dt);

			InputDevice *getDevice(InputDeviceBinding binding);
			InputDevice *getDevice(Binding b);

			Keyboard *getKeyboard();
			Mouse *getMouse();
			Controller *getController(u32 i);

		protected:

			std::unordered_map<InputDeviceBinding, InputDevice*> devices;

		};

	}
}