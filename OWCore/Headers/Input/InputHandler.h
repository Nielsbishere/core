#pragma once
#include "Binding.h"
#include <Types/Vector.h>
#include <bitset>
#include <array>
namespace oi {
	namespace wc {

		struct InputValues {
			std::bitset<Key::length> keys;
			std::bitset<Click::length> mouseButtons;
			std::bitset<ControllerButton::length * Controller::maxControllers> cButtons;
			std::array<f32, ControllerAxis::length * Controller::maxControllers> cAxis;
			std::bitset<Controller::maxControllers> controllers;
		};

		class Window;

		class InputHandler {

			friend class Window;

		public:

			void update(Window &w);
			void updateController(u32 i, Window &w);

			bool isDown(Binding b);
			bool isUp(Binding b);
			bool isPressed(Binding b);
			bool isReleased(Binding b);

			f32 getAxis(Binding b);

			//Check if controller is connected
			bool isConnected(u32 id);

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