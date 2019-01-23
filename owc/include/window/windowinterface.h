#pragma once

#include "types/vector.h"
#include "utils/timer.h"
#include "input/binding.h"
#include "input/inputdevice.h"

namespace oi {

	namespace wc {

		class Window;
		class InputHandler;
		class InputManager;
		class InputDevice;
		class Keyboard;
		class Mouse;
		class Controller;

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
			
			virtual void init(){}							//Called when the window is intialized once
			
			virtual void initSurface(Vec2u) { }	//Called when the window is intialized
			virtual void destroySurface(){}

			virtual void onResize(Vec2u) { }
			virtual void onAspectChange(f32) { }
			virtual void onMove(Vec2i) { }
			virtual void onInput(InputDevice*, Binding, bool) { }

			virtual void onMouseMove(Vec2) { }
			virtual void onMouseWheel(f32) { }
			virtual void onMouseDrag(Vec2) { }
			
			virtual void update(f32 delta) { runtime += delta; }
			virtual void render() {}

			virtual void setFocus(bool) { }

			Window *getParent();
			InputHandler &getInputHandler();
			InputManager &getInputManager();

			Keyboard *getKeyboard();
			Mouse *getMouse();
			Controller *getController(u32 id);

			Timer getTimer() { return timer; }
			f64 getDuration() { return timer.getDuration(); }

			f64 getRuntime() { return runtime; }

		private:

			Window *parent;
			Timer timer;
			float runtime = 0.f;

		};

	}

}