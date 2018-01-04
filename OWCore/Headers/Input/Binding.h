#pragma once
#include "Key.h"
namespace oi {
	namespace wc {

		enum class BindingType {

			KEYBOARD,
			MOUSE,
			CONTROLLER_BUTTON,
			CONTROLLER_AXIS,

			UNDEFINED = 0xFF

		};

		class Binding {

		public:

			Binding(Key k);

			//Get as uint
			operator u32();

			BindingType getType();
			u32 getControllerId();
			u32 getCode();

			operator Key();				//Returns Key if possible

		private:

			u8 bindingType;
			u8 controllerId;
			u16 code;
		};

	}
}