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
			MISC
		};

		class InputHandler {

		public:
		
			struct IDBHash {
				size_t operator()(const InputDeviceBinding &idb) const { return (size_t) idb; }
			};

			void init();
			void update(Window *w, f32 dt);

			InputDevice *getDevice(InputDeviceBinding binding);
			InputDevice *getDevice(Binding b);

			Keyboard *getKeyboard();
			Mouse *getMouse();
			Controller *getController(u32 i);

			u32 getControllers();

		protected:
		
			std::unordered_map<InputDeviceBinding, InputDevice*, IDBHash> devices;

		};

	}
}