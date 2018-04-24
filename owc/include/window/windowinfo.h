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
			friend class WindowManager;

		public:

			WindowInfo(String title, u32 version, void *handle = nullptr, bool inFocus = true);

			String getTitle() const;
			Vec2i getPosition() const;
			Vec2u getSize() const;
			Window *getParent() const;

			void move(Vec2i newPos);
			void resize(Vec2u newSize);

			void focus();

			bool isInFocus() const;
			u32 getVersion() const;
			
			void _forceSize(Vec2u size);

		protected:

			void setParent(Window *w);
			void notify(WindowAction action);

		private:

			Vec2i position = {};
			Vec2u size = {};
			WindowAction pending;
			bool inFocus;

			u32 version;
			String title;
			Window *parent;
			void *handle;

		};

	}

}