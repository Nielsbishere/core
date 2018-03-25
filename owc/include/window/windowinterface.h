#pragma once

#include <types/vector.h>
#include "input/binding.h"
#include "input/inputdevice.h"

namespace oi {

	namespace wc {

		class Window;
		class InputHandler;
		class InputManager;
		class InputDevice;

		class WindowInterface {

			friend class Window;

		public:

			virtual ~WindowInterface() {}

			virtual void init() {}

			virtual void onResize(Vec2u size) {}
			virtual void onMove(Vec2i pos) {}
			virtual void onInput(InputDevice *device, Binding b, bool isDown) {}

			virtual void onMouseMove(Vec2 pos) {}
			virtual void onMouseWheel(flp delta) {}

			virtual void update(flp delta) {}
			virtual void render() {}
			virtual void repaint() { render(); }

			virtual void setFocus(bool isFocussed) {}
			virtual void onConnect(InputDevice *device) {}
			virtual void onDisconnect(InputDevice *device) {}

			Window *getParent();
			InputHandler &getInputHandler();
			InputManager &getInputManager();

		private:

			Window *parent;

		};

	}

}