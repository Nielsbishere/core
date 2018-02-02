#include "Input/InputHandler.h"
#include "Window/Window.h"
using namespace oi::wc;
using namespace oi;

#ifdef __WINDOWS__

#include <Xinput.h>
#pragma comment(lib, "XInput.lib")

void InputHandler::updateController(u32 i, Window &w) {

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	u32 off = i * ControllerAxis::length;

	if (nw.controllers[i] = XInputGetState(i, &state) == ERROR_SUCCESS) {

		nw.cAxis[off + ControllerAxis("Lx").getIndex()] = state.Gamepad.sThumbLX / 32767.f;
		nw.cAxis[off + ControllerAxis("Ly").getIndex()] = state.Gamepad.sThumbLY / 32767.f;

		nw.cAxis[off + ControllerAxis("Rx").getIndex()] = state.Gamepad.sThumbRX / 32767.f;
		nw.cAxis[off + ControllerAxis("Ry").getIndex()] = state.Gamepad.sThumbRY / 32767.f;

		nw.cAxis[off + ControllerAxis("L2").getIndex()] = state.Gamepad.bLeftTrigger / 255.f;
		nw.cAxis[off + ControllerAxis("R2").getIndex()] = state.Gamepad.bRightTrigger / 255.f;

		off = i * ControllerButton::length;

		nw.cButtons[off + ControllerButton("Cross").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
		nw.cButtons[off + ControllerButton("Square").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
		nw.cButtons[off + ControllerButton("Triangle").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
		nw.cButtons[off + ControllerButton("Circle").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;

		nw.cButtons[off + ControllerButton("Down").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
		nw.cButtons[off + ControllerButton("Left").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
		nw.cButtons[off + ControllerButton("Up").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
		nw.cButtons[off + ControllerButton("Right").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

		nw.cButtons[off + ControllerButton("L1").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		nw.cButtons[off + ControllerButton("R1").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		nw.cButtons[off + ControllerButton("L3").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
		nw.cButtons[off + ControllerButton("R3").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;

		nw.cButtons[off + ControllerButton("Options").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
		nw.cButtons[off + ControllerButton("Share").getIndex()] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;

	} else {
		memset(&nw.cAxis[off], 0, 4 * ControllerAxis::length);

		off = i * ControllerButton::length;

		for (u32 j = 0; j < ControllerButton::length; ++j)
			nw.cButtons[off + j] = false;
	}

	for (u32 j = 0; j < ControllerButton::length; ++j) {

		if (!old.cButtons[off + j] && nw.cButtons[off + j])
			if (w.wi != nullptr)
				w.wi->onKeyPress(Binding(ControllerButton::get(j), i));

		if (old.cButtons[off + j] && !nw.cButtons[off + j])
			if (w.wi != nullptr)
				w.wi->onKeyRelease(Binding(ControllerButton::get(j), i));

	}
}

#endif

void InputHandler::update(Window &w) {
	old = nw;

	for (u32 i = 0; i < Controller::maxControllers; ++i)
		updateController(i, w);
}

void InputHandler::handleKey(Key k, bool value) {
	if(k.getIndex() < Key::length)
		nw.keys[k.getIndex()] = value;
}

void InputHandler::handleClick(Click c, bool value) {
	if (c.getIndex() < Click::length)
		nw.mouseButtons[c.getIndex()] = value;
}

bool InputHandler::isDown(Binding b) {

	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && old.keys[b.getCode()];

	if (b.getType() == BindingType::MOUSE)
		return nw.mouseButtons[b.getCode()] && old.mouseButtons[b.getCode()];

	if (b.getType() == BindingType::CONTROLLER_BUTTON) {
		u32 i = b.getControllerId() * ControllerButton::length + b.getCode();
		return nw.cButtons[i] && old.cButtons[i];
	}

	if (b.getType() == BindingType::CONTROLLER_AXIS) {
		u32 i = b.getControllerId() * ControllerAxis::length + b.getCode();
		return abs(nw.cAxis[i]) > 1.f / 3.f && abs(old.cAxis[i]) > 1.f / 3.f;
	}

	return false;
}

bool InputHandler::isUp(Binding b) {

	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && !old.keys[b.getCode()];

	if (b.getType() == BindingType::MOUSE)
		return !nw.mouseButtons[b.getCode()] && !old.mouseButtons[b.getCode()];

	if (b.getType() == BindingType::CONTROLLER_BUTTON) {
		u32 i = b.getControllerId() * ControllerButton::length + b.getCode();
		return !nw.cButtons[i] && !old.cButtons[i];
	}

	if (b.getType() == BindingType::CONTROLLER_AXIS) {
		u32 i = b.getControllerId() * ControllerAxis::length + b.getCode();
		return abs(nw.cAxis[i]) < 1.f / 3.f && abs(old.cAxis[i]) < 1.f / 3.f;
	}

	return false;
}

bool InputHandler::isPressed(Binding b) {

	if (b.getType() == BindingType::KEYBOARD)
		return nw.keys[b.getCode()] && !old.keys[b.getCode()];

	if (b.getType() == BindingType::MOUSE)
		return nw.mouseButtons[b.getCode()] && !old.mouseButtons[b.getCode()];

	if (b.getType() == BindingType::CONTROLLER_BUTTON) {
		u32 i = b.getControllerId() * ControllerButton::length + b.getCode();
		return nw.cButtons[i] && !old.cButtons[i];
	}

	if (b.getType() == BindingType::CONTROLLER_AXIS) {
		u32 i = b.getControllerId() * ControllerAxis::length + b.getCode();
		return abs(nw.cAxis[i]) > 1.f / 3.f && abs(old.cAxis[i]) < 1.f / 3.f;
	}

	return false;
}

bool InputHandler::isReleased(Binding b) {

	if (b.getType() == BindingType::KEYBOARD)
		return !nw.keys[b.getCode()] && old.keys[b.getCode()];

	if (b.getType() == BindingType::MOUSE)
		return !nw.mouseButtons[b.getCode()] && old.mouseButtons[b.getCode()];

	if (b.getType() == BindingType::CONTROLLER_BUTTON) {
		u32 i = b.getControllerId() * ControllerButton::length + b.getCode();
		return !nw.cButtons[i] && old.cButtons[i];
	}

	if (b.getType() == BindingType::CONTROLLER_AXIS) {
		u32 i = b.getControllerId() * ControllerAxis::length + b.getCode();
		return abs(nw.cAxis[i]) < 1.f / 3.f && abs(old.cAxis[i]) > 1.f / 3.f;
	}

	return false;
}

f32 InputHandler::getAxis(Binding b) {

	if (b.getType() == BindingType::CONTROLLER_AXIS)
		return nw.cAxis[b.getControllerId() * ControllerAxis::length + b.getCode()];

	return isDown(b) ? 1.f : 0.f;
}

Vec2 InputHandler::getCursor(Window &w) {
	return w.getInfo().getCursor();
}

bool InputHandler::isConnected(u32 id) {
	return nw.controllers[id];
}