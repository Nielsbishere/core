#pragma once
#include "Key.h"
#include "Binding.h"
#include <Types/Vector.h>
#include <bitset>
namespace oi {
	namespace wc {

		struct InputValues {
			std::bitset<Key::length> keys;
			std::bitset<Click::length> mouseButtons;
		};

		class Window;

		class InputHandler {

			friend class Window;

		public:

			void update();

			bool isDown(Binding b);
			bool isUp(Binding b);
			bool isPressed(Binding b);
			bool isReleased(Binding b);

			//Get the click in normalized window space
			Vec2 getCursor(Window &w);

		protected:

			void handleKey(Key k, bool value);
			void handleClick(Click c, bool value);

		private:

			InputValues old, nw;

		};

	}
}