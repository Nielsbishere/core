#pragma once
#include "types/string.h"
#include "types/vector.h"
#include "windowaction.h"
#include "platforms/generic.h"

namespace oi {

	namespace wc {

		class Window;

		class WindowInfo {

			friend class Window;
			friend struct WindowExt;
			friend class WindowManager;

		public:

			WindowInfo(String title, u32 version, WindowHandleExt *handle = nullptr, bool inFocus = true);

			String getTitle() const;
			Vec2i getPosition() const;
			Vec2u getSize() const;
			Window *getParent() const;
			WindowHandleExt *getParam() const;
			bool isFullScreen() const;

			void toggleFullScreen();
			void setFullScreen(bool fullScreen);

			void focus();

			bool isInFocus() const;
			bool isMinimized() const;
			u32 getVersion() const;

		protected:

			void setParent(Window *w);
			void notify(WindowAction action);

		private:

			Vec2i position = {};
			Vec2u size = {};
			WindowAction pending;
			bool inFocus, fullScreen = false, minimized = false, flipped = false, flippedOnStart = false;

			u32 version;
			String title;
			Window *parent;
			WindowHandleExt *handle;

		};

	}

}