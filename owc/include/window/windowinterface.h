#pragma once

#include <types/vector.h>
#include "input/binding.h"
#include "input/inputdevice.h"
#include <utils/timer.h>

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

			//!Important! load and save from file have to be implemented
			//This is because other devices (mainly mobile) may terminate your interface
			//Afterwards, your app should be loaded from the file specified
			virtual void load(String path) = 0;
			
			//!Important! load and save from file have to be implemented
			//This is because other devices (mainly mobile) may terminate your interface
			//When that happens, this function will save to the file specified
			virtual void save(String path) = 0;
			
			virtual void init(){}			//Called when the window is intialized once
			
			virtual void initSurface() {}	//Called when the window is intialized
			virtual void destroySurface(){}

			virtual void onResize(Vec2u size) {}
			virtual void onMove(Vec2i pos) {}
			virtual void onInput(InputDevice *device, Binding b, bool isDown) {}

			virtual void onMouseMove(Vec2 pos) {}
			virtual void onMouseWheel(flp delta) {}
			virtual void onMouseDrag(Vec2 dpos) {}
			
			virtual void update(flp delta) {}
			virtual void render() {}

			virtual void setFocus(bool isFocussed) {}

			Window *getParent();
			InputHandler &getInputHandler();
			InputManager &getInputManager();

			Timer getTimer() { return t; }
			f64 getDuration() { return t.getDuration(); }

		private:

			Window *parent;
			Timer t;

		};

	}

}