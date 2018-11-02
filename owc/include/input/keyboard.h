#pragma once

#include "inputdevice.h"
#include "types/bitset.h"

namespace oi {

	namespace wc {

		class Keyboard : public InputDevice {

			friend struct WindowExt;

		public:

			Keyboard();
			InputState getState(Binding b) const override;
			f32 getAxis(Binding b) const override;

			void update(Window *w, f32 dt) override;

		protected:

			void update(Binding b, bool down) override;

			StaticBitset<Key::length - 1> prev, next;

		};
	}

}