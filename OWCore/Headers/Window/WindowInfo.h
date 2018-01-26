#pragma once
#include <Types/OString.h>
#include <Types/Vector.h>
#include "WindowAction.h"

namespace oi {
	namespace wc {

		class Window;

		class WindowInfo {

			friend class Window;
			friend struct WindowHandle;

		public:

			WindowInfo(OString title, Vec2u size = Vec2u(480, 270), Vec2i position = Vec2i(0, 0), bool inFocus = true);

			OString getTitle();
			Vec2i getPosition();
			Vec2u getSize();
			Vec2 getCursor();
			Window *getParent();

			void move(Vec2i newPos);
			void resize(Vec2u newSize);

			void focus();

			bool isInFocus();

		protected:

			void initParent(Window *w);
			void notifyUpdate(WindowAction action);

			void moveCursor(Vec2u pos);

		private:

			Vec2i position;
			Vec2u size, cursor;
			bool inFocus;
			WindowAction pending;

			OString title;
			Window *parent;
		};

	}
}