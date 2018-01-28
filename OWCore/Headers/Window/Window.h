#pragma once
#include "WindowHandle.h"
#include "WindowInfo.h"
#include "Input/Binding.h"
#include "Input/InputHandler.h"
#include "Input/InputManager.h"
#include "WindowInterface.h"
#include <Utils/Timer.h>

namespace oi {
	namespace wc {

		class Window {

			friend class WindowInfo;
			friend struct WindowHandle;
			friend class InputHandler;

		public:

			static Window &create(WindowInfo info);
			static Window *getByHandle(WindowHandle handle);

			static void waitAll();		//Wait for all windows to close

			~Window();
			void destroy();
			void setInterface(WindowInterface *wi);
			void swapBuffers();

			bool isActivated();
			WindowInfo &getInfo();
			WindowHandle &getHandle();
			InputHandler &getInput();
			InputManager &getInputManager();
			WindowInterface *getInterface();

			static constexpr u32 maxWindows = 32;


		protected:

			void handleBinding(Key key, bool value);
			void handleBinding(Click click, bool value);

			static void updateAll();	//Update all windows
			static void initAll();		//Initialize all windows
			void update();
			void init();

			void notifyUpdate(WindowAction pending);

			Window();

			void activate();
			u32 getIndex();

		private:

			WindowInfo info;
			WindowHandle handle;
			InputHandler input;
			InputManager inputManager;
			WindowInterface *wi;

			bool activated;
			u32 index;

			Timer t;
			f64 lastTick;

			static Window windows[maxWindows];
		};
	}
}