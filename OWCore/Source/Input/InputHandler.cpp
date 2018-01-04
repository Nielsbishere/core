#include "Input/InputHandler.h"
using namespace oi::wc;
using namespace oi;

void InputHandler::update() {
	old = nw;
}

void InputHandler::handleKey(Key k, bool value) {
	if(k.getIndex() < Key::size())
		nw.keys[k.getIndex()] = value;
}

bool InputHandler::isDown(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && old.keys[b.getCode()];

	return false;
}

bool InputHandler::isUp(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && !old.keys[b.getCode()];

	return false;
}

bool InputHandler::isPressed(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && !old.keys[b.getCode()];

	return false;
}

bool InputHandler::isReleased(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && old.keys[b.getCode()];

	return false;
}