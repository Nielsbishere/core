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
			
			virtual void init(){}							//Called when the window is intialized once
			
			virtual void initSurface(Vec2u size) { size; }	//Called when the window is intialized
			virtual void destroySurface(){}

			virtual void onResize(Vec2u size) { size; }
			virtual void onAspectChange(f32 aspect) { aspect; }
			virtual void onMove(Vec2i pos) { pos; }
			virtual void onInput(InputDevice *device, Binding b, bool isDown) { device; b; isDown; }

			virtual void onMouseMove(Vec2 pos) { pos; }
			virtual void onMouseWheel(f32 delta) { delta; }
			virtual void onMouseDrag(Vec2 dpos) { dpos; }
			
			virtual void update(f32 delta) { runtime += delta; }
			virtual void render() {}

			virtual void setFocus(bool isFocussed) { isFocussed; }

			Window *getParent();
			InputHandler &getInputHandler();
			InputManager &getInputManager();

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