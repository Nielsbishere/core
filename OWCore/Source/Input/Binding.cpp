#include "Input/Binding.h"
#include <Utils/Log.h>
using namespace oi::wc;
using namespace oi;

Binding::Binding(Key k): bindingType((u8)BindingType::KEYBOARD), code((u16)k.getIndex()), controllerId(0) { }

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

Binding::operator u32() {
	return ((u32)controllerId << 24) | ((u32)bindingType << 16) | (u32)code;
}