#pragma once
#include "windowinfo.h"
#include "input/inputmanager.h"

namespace oi {

	namespace wc {

		class WindowInterface;
		class WindowManager;

		class Window {

			friend class WindowInfo;
			friend class WindowManager;
			friend class InputHandler;
			friend struct Window_imp;

		public:

			void setInterface(WindowInterface *wi);

			WindowInfo &getInfo();
			InputHandler &getInputHandler();
			InputManager &getInputManager();
			WindowInterface *getInterface();
			WindowManager *getParent();

		protected:

			void update();
			void updatePlatform();
			void init();
			void initPlatform();
			void destroyPlatform();
			void swapBuffers();

			Window(WindowManager *parent, WindowInfo info);
			~Window();

		private:

			WindowManager *parent;
			WindowInfo info;
			InputHandler inputHandler;
			InputManager inputManager;
			WindowInterface *wi = nullptr;

			flp lastTick = (flp) 0;

			u8 platformData[8192];	//Reserved for per platform implementation

		};
	}

}