#pragma once

#include "inputdevice.h"
#include "types/bitset.h"

namespace oi {

	namespace wc {

		class Mouse : public InputDevice {

			friend struct WindowExt;

		public:

			Mouse();
			InputState getState(Binding b) const override;
			f32 getAxis(Binding b) const override;

			void update(Window *w, f32 dt) override;

		protected:

			void update(Binding b, bool down) override;

			StaticBitset<MouseButton::length - 1> prev, next;
			f32 axes[MouseAxis::length - 1];

		};
	}

}