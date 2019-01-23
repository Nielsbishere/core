#pragma once
#include "enums.h"

namespace oi {

	namespace wc {

		enum class BindingType {

			UNDEFINED,
			KEYBOARD,
			MOUSE_BUTTON,
			MOUSE_AXIS,
			CONTROLLER_BUTTON,
			CONTROLLER_AXIS

		};

		enum class InputType {
			MOUSE, KEYBOARD, CONTROLLER
		};

		class Binding {

		public:

			Binding();
			Binding(const Key_s k, bool delta = false);
			Binding(const MouseButton_s c, bool delta = false);
			Binding(const MouseAxis_s ma, bool delta = false);
			Binding(const ControllerButton_s cb, u8 controllerId = 0, bool delta = false);
			Binding(const ControllerAxis_s ca, u8 controllerId = 0, bool delta = false);

			Binding(String name);
			Binding(u32 value);

			u32 toUInt() const;

			BindingType getBindingType() const;
			InputType getInputType() const;
			u32 getControllerId() const;
			u32 getCode() const;
			bool useDelta() const;

			Key toKey() const;
			MouseButton toMouseButton() const;
			MouseAxis toMouseAxis() const;
			ControllerButton toButton() const;
			ControllerAxis toAxis() const;

			String toString() const;

		private:

			u8 bindingType;			//flag 0x80: use delta
			u8 controllerId = 0;
			u16 code;

		};

	}
}