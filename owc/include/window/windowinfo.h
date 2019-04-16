#pragma once
#include "types/string.h"
#include "types/vector.h"

namespace oi {

	namespace wc {

		class Window;
		class WindowViewport;
		struct WindowViewportExt;
		struct AppExt;

		class WindowInfo {

			friend class WindowViewport;
			friend struct WindowViewportExt;
			friend class WindowManager;

		public:

			WindowInfo(AppExt *app, bool decorated = false);

			Vec2i getPosition() const;
			Window *getParent() const;
			AppExt *getApp() const;

			bool isInFocus() const;
			bool isMinimized() const;
			bool isDecorated() const;

		protected:

			void setParent(Window *w);

		private:

			Vec2i position{};
			bool inFocus = true, minimized = false, decorated = false;

			Window *parent;
			AppExt *handle;

		};

	}

}