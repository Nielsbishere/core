#pragma once
#include "../viewport/viewport.hpp"
#include "input/inputmanager.h"

namespace oi {

	namespace wc {

		class WindowInterface;

		class Window {

		public:

			template<typename T, typename ...args>
			static Window *create(const String &title, const u32 version, const args&... arg);
			static Window *get() { return window; }

			void setInterface(WindowInterface *wi);

			InputHandler &getInputHandler() { return inputHandler; }
			InputManager &getInputManager() { return inputManager; }
			WindowInterface *getInterface() { return wi; }
			const String &getTitle() { return title; }
			const u32 getVersion() { return version; }

			template<typename T = Viewport>
			T *getViewport();

			~Window();

			Window(const Window&) = delete;
			Window(Window&&) = delete;
			Window &operator=(const Window&) = delete;
			Window &operator=(Window&&) = delete;

			void wait();

		protected:

			bool update();

			Window(const String &title, const u32 version);

			template<typename T, typename ...args>
			void setViewport(const args&... arg);

		private:

			static Window *window;

			InputHandler inputHandler;
			InputManager inputManager;

			String title;

			WindowInterface *wi = nullptr;
			Viewport *viewport = nullptr;

			u32 version = 0;

		};

		template<typename T, typename ...args>
		static Window *Window::create(const String &title, const u32 version, const args&... arg) {

			if (window)
				Log::throwError<Window, 0x0>("There can only be one main window");

			window = new Window(title, version);
			window->setViewport<T>(arg...);
			return window;
		}

		template<typename T, typename ...args>
		void Window::setViewport(const args&... arg) {
			static_assert(std::is_base_of<Viewport, T>::value && !std::is_same<Viewport, T>::value, "Window::setViewport requires a Viewport subclass");
			destroyPointer(viewport);
			viewport = new T(arg...);
			viewport->init(this);
		}

		template<typename T>
		T *Window::getViewport() {
			static_assert(std::is_base_of<Viewport, T>::value, "Window::getViewport requires a Viewport subclass");
			return dynamic_cast<T*>(viewport);
		}

	}

}