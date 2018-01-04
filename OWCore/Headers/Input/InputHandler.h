#pragma once
#include "Key.h"
#include "Binding.h"
#include <bitset>
namespace oi {
	namespace wc {

		struct InputValues {
			std::bitset<Key::length> keys;
		};

		class InputHandler {

		public:

			void update();

			void handleKey(Key k, bool value);

			bool isDown(Binding b);
			bool isUp(Binding b);
			bool isPressed(Binding b);
			bool isReleased(Binding b);

		private:

			InputValues old, nw;

		};

	}
}