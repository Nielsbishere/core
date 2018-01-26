#pragma once
#include "Key.h"
#include "Click.h"
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
			Binding(Click c);

			//Get as uint
			operator u32();

			BindingType getType();
			u32 getControllerId();
			u32 getCode();

			Key toKey();				//Returns Key if possible
			Click toClick();			//Returns Click if possible

			OString toString();

		private:

			u8 bindingType;
			u8 controllerId;
			u16 code;
		};

	}
}