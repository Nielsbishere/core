#pragma once

#include "binding.h"
#include <functional>

namespace oi {

	namespace wc {

		enum class InputState {
			ANY, UP, PRESSED, DOWN, RELEASED
		};

		class InputDevice;
		class Window;
		
		//Callback function for input
		//@param InputDevice*; which device has changed input
		//@param Binding; which key has changed input state
		//@param InputState; to which state it has changed
		typedef std::function<void (InputDevice*, Binding, InputState)> InputCallbackFunc;

		//Anything input related
		//Thanks to /velddev for the concept
		class InputDevice {

		public:

			InputDevice(InputType type, u32 id = 0);
			virtual ~InputDevice();

			bool isConnected() const { return connected; }

			bool isDown(Binding b) const;
			bool isUp(Binding b) const;
			bool isPressed(Binding b) const;
			bool isReleased(Binding b) const;

			flp toAxis(InputState state) const;

			virtual InputState getState(Binding b) const = 0;

			virtual flp getAxis(Binding b) const = 0;
			virtual void update(Window *w, flp dt) = 0;
			virtual void update(Binding b, bool down) {}

			const InputType getType() const;
			u32 getId() const;

		protected:

			bool connected = true, prevConnected = false;

		private:

			InputType type;
			u32 id;
		};

	}

}