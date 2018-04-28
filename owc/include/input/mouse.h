#pragma once

#include "inputdevice.h"
#include <bitset>

namespace oi {

	namespace wc {

		class Mouse : public InputDevice {

			friend struct WindowExt;

		public:

			Mouse();
			InputState getState(Binding b) const override;
			flp getAxis(Binding b) const override;

			void update(Window *w, flp dt) override;

		protected:

			void update(Binding b, bool down) override;

			std::bitset<MouseButton::length - 1> prev, next;
			flp axes[MouseAxis::length - 1];

		};
	}

}