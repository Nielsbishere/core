#pragma once

#include <Types/Vector.h>
#include "Input/Binding.h"

namespace oi {

	namespace wc {

		class Window;

		class WindowInterface {

			friend class Window;

		public:

			virtual ~WindowInterface() {}

			virtual void init() {}

			virtual void onResize(Vec2u size) {}
			virtual void onMove(Vec2i pos) {}
			virtual void onKeyPress(Binding b) {}
			virtual void onKeyRelease(Binding b) {}

			virtual void onMouseMove(Vec2 pos) {}
			virtual void onMouseExit(Vec2 pos) {}
			virtual void onMouseEnter(Vec2 pos) {}

			virtual void update(f64 delta) {}
			virtual void render() {}
			virtual void repaint() { render(); }

			virtual void setFocus(bool isFocussed) {}

			Window *getParent() { return parent; }

		private:

			Window *parent;

		};

	}

}