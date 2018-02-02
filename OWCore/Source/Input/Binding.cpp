#include "Input/Binding.h"
#include <Utils/Log.h>
using namespace oi::wc;
using namespace oi;

Binding::Binding(const Key_s k): bindingType((u8)BindingType::KEYBOARD), code((u16)Key(k).getIndex()), controllerId(0) { }
Binding::Binding(const Click_s c) : bindingType((u8)BindingType::MOUSE), code((u16)Click(c).getIndex()), controllerId(0) { }
Binding::Binding(const ControllerButton_s cb, u32 _controllerId) : bindingType((u8)BindingType::CONTROLLER_BUTTON), code((u16)ControllerButton(cb).getIndex()), controllerId(_controllerId) {}
Binding::Binding(const ControllerAxis_s cb, u32 _controllerId) : bindingType((u8)BindingType::CONTROLLER_AXIS), code((u16)ControllerAxis(cb).getIndex()), controllerId(_controllerId) {}

BindingType Binding::getType() {
	return (BindingType)bindingType;
}

u32 Binding::getControllerId() {
	return (u32)controllerId;
}
u32 Binding::getCode() {
	return (u32)code;
}

Key Binding::toKey() {
	if (getType() == BindingType::KEYBOARD)
		return Key::get(getCode());
	return Key::get(0);
}

Click Binding::toClick() {
	if (getType() == BindingType::MOUSE)
		return Click::get(getCode());
	return Click::get(0);
}

ControllerButton Binding::toButton() {
	if (getType() == BindingType::CONTROLLER_BUTTON)
		return ControllerButton::get(getCode());
	return ControllerButton::get(0);
}

ControllerAxis Binding::toAxis() {
	if (getType() == BindingType::CONTROLLER_AXIS)
		return ControllerAxis::get(getCode());
	return ControllerAxis::get(0);
}

Binding::operator u32() {
	return ((u32)controllerId << 24) | ((u32)bindingType << 16) | (u32)code;
}

OString Binding::toString() {

	if (getType() == BindingType::KEYBOARD)
		return toKey().getName() + " key";

	if (getType() == BindingType::MOUSE)
		return toClick().getName() + " mouse";

	if (getType() == BindingType::CONTROLLER_BUTTON)
		return toButton().getName() + " button #" + controllerId;

	if (getType() == BindingType::CONTROLLER_AXIS)
		return toAxis().getName() + " axis #" + controllerId;

	return "Undefined";
}

Binding::Binding(OString ostr) {

	auto split = ostr.split(" ");

	controllerId = 0;

	if (split.size() != 1) {

		OString end = split[split.size() - 1];

		if (end.equalsIgnoreCase("key")) {

			bindingType = (u8)BindingType::KEYBOARD;
			code = (u8)Key(split[0]).getIndex();

		} else if (end.equalsIgnoreCase("mouse")) {

			bindingType = (u8)BindingType::MOUSE;
			code = (u8)Click(split[0]).getIndex();

		} else if (split.size() == 3) {

			u32 num = (u32)end.cutBegin(1).toLong();

			if (num >= Controller::maxControllers)
				goto failed;

			controllerId = num;

			if (split[1].equalsIgnoreCase("button")) {

				bindingType = (u8)BindingType::CONTROLLER_BUTTON;
				code = (u8)ControllerButton(split[0]).getIndex();

			} else if(split[1].equalsIgnoreCase("axis")){

				bindingType = (u8)BindingType::CONTROLLER_AXIS;
				code = (u8)ControllerAxis(split[0]).getIndex();
			}
		}

		goto success;
	} 

	failed:
	code = 0;
	bindingType = 0xFFU;

success:
	;
}