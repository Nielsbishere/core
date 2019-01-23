#pragma once

#include "platforms/generic.h"
#include "inputdevice.h"
#include "types/vector.h"
#include "types/bitset.h"

namespace oi {

	namespace wc {

		class Controller : public InputDevice {
			
			friend struct WindowExt;

		public:

			Controller(u32 id);
			~Controller();
			InputState getState(Binding b) const override;
			f32 getAxis(Binding b, bool delta) const override;

			void update(Window *w, f32 dt) override;
			void vibrate(Vec2 amount, f32 time);

		protected:

			void update(Binding b, bool down) override;

			StaticBitset<ControllerButton::length - 1> prev, next;
			f32 vibrationTime = 0;

			static constexpr u32
				axisCount = ControllerAxis::length - 1,
				prevAxes = axisCount,
				deltaAxes = axisCount * 2,
				axesSize = axisCount * u32(sizeof(f32));

			f32 axes[axisCount * 3];

		};
	}

}