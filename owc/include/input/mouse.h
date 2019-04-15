#pragma once

#include "inputdevice.h"
#include "types/bitset.h"

namespace oi {

	namespace wc {

		class Mouse : public InputDevice {

			friend struct WindowViewportExt;

		public:

			Mouse();
			InputState getState(Binding b) const override;
			f32 getAxis(Binding b, bool delta) const override;

			void update(Window *w, f32 dt) override;

		protected:

			void update(Binding b, bool down) override;

			static constexpr u32 
				axisCount = MouseAxis::length - 1, 
				prevAxes = axisCount, 
				deltaAxes = axisCount * 2,
				axesSize = axisCount * u32(sizeof(f32));

			StaticBitset<MouseButton::length - 1> prev, next;
			f32 axes[axisCount * 3];

		};
	}

}