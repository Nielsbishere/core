#pragma once

#include "window.h"

namespace oi {

	namespace wc {

		class WindowManager {

			friend class Window;

		public:
			
			static WindowManager *get();

			void waitAll();

			Window *create(WindowInfo info);
			void remove(Window *w);

			u32 getWindows();
			Window *operator[](u32 i);

			WindowManager();
			~WindowManager();

		protected:

			void updateAll();
			void initAll();

		private:

			static WindowManager *instance;
			std::vector<Window*> windows;
		};

	}

}