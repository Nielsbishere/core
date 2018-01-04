#pragma once
#include "WindowHandle.h"
#include "WindowInfo.h"
#include "Input/Binding.h"
#include "Input/InputHandler.h"
#include "WindowInterface.h"
#include <Utils/Timer.h>

namespace oi {
	namespace wc {

		class Window {

			friend class WindowInfo;
			friend struct WindowHandle;

		public:

			static Window &create(WindowInfo info);
			static Window *getByHandle(WindowHandle handle);

			static void waitAll();		//Wait for all windows to close

			~Window();
			void destroy();
			void handleBinding(Key key, bool value);
			void setInterface(WindowInterface *wi);
			void swapBuffers();

			bool isActivated();
			WindowInfo &getInfo();
			WindowHandle &getHandle();
			InputHandler &getInput();
			WindowInterface *getInterface();

			static constexpr u32 maxWindows = 32;


		protected:

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
			WindowInterface *wi;

			bool activated;
			u32 index;

			Timer t;
			f64 lastTick;

			static Window windows[maxWindows];
		};
	}
}