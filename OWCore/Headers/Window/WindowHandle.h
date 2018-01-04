#pragma once
#include <Template/PlatformDefines.h>
#include "WindowAction.h"

namespace oi {
	namespace wc {

		class WindowInfo;

		struct WindowHandle {
			#ifdef __WINDOWS__
				HWND window;
				HINSTANCE instance;
			#endif

			//Create a window on the native device
			static bool instantiate(WindowHandle &handle, WindowInfo &info);

			//Check if the window has the same id (Varies per system; windows uses 'HWND' as id)
			bool sameId(const WindowHandle &other);

			void destroy();
			void update(WindowInfo &info, WindowAction action);
			void swapBuffers();

			#ifdef __WINDOWS__
				static LRESULT CALLBACK windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			#endif
		};

	}
}