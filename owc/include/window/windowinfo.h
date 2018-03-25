#pragma once
#include <types/string.h>
#include <types/vector.h>
#include "windowaction.h"

namespace oi {

	namespace wc {

		class Window;

		class WindowInfo {

			friend class Window;
			friend struct Window_imp;

		public:

			WindowInfo(String title, Vec2u size = Vec2u(480, 270), Vec2i position = Vec2i(0, 0), bool inFocus = true);

			String getTitle() const;
			Vec2i getPosition() const;
			Vec2u getSize() const;
			Window *getParent() const;

			void move(Vec2i newPos);
			void resize(Vec2u newSize);

			void focus();

			bool isInFocus() const;

		protected:

			void setParent(Window *w);
			void notify(WindowAction action);

		private:

			Vec2i position;
			Vec2u size;
			WindowAction pending;
			bool inFocus;

			String title;
			Window *parent;

		};

	}

}