#pragma once

#include "inputdevice.h"
#include <bitset>

namespace oi {

	namespace wc {

		class Keyboard : public InputDevice {

			friend struct Window_imp;

		public:

			Keyboard();
			InputState getState(Binding b) const override;
			flp getAxis(Binding b) const override;

			void update(Window *w, flp dt) override;

		protected:

			void update(Binding b, bool down) override;

			std::bitset<Key::length - 1> prev, next;

		};
	}

}