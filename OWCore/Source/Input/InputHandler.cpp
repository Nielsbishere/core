#include "Input/InputHandler.h"
#include "Window/Window.h"
using namespace oi::wc;
using namespace oi;

void InputHandler::update() {
	old = nw;
}

void InputHandler::handleKey(Key k, bool value) {
	if(k.getIndex() < Key::size())
		nw.keys[k.getIndex()] = value;
}

void InputHandler::handleClick(Click c, bool value) {
	if (c.getIndex() < Click::size())
		nw.mouseButtons[c.getIndex()] = value;
}

bool InputHandler::isDown(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && old.keys[b.getCode()];
	if (b.getType() == BindingType::MOUSE)
		return nw.mouseButtons[b.getCode()] && old.mouseButtons[b.getCode()];

	return false;
}

bool InputHandler::isUp(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && !old.keys[b.getCode()];
	if (b.getType() == BindingType::MOUSE)
		return !nw.mouseButtons[b.getCode()] && !old.mouseButtons[b.getCode()];

	return false;
}

bool InputHandler::isPressed(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && !old.keys[b.getCode()];
	if (b.getType() == BindingType::MOUSE)
		return nw.mouseButtons[b.getCode()] && !old.mouseButtons[b.getCode()];

	return false;
}

bool InputHandler::isReleased(Binding b) {
	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && old.keys[b.getCode()];
	if (b.getType() == BindingType::MOUSE)
		return !nw.mouseButtons[b.getCode()] && old.mouseButtons[b.getCode()];

	return false;
}

Vec2 InputHandler::getCursor(Window &w) {
	return w.getInfo().getCursor();
}