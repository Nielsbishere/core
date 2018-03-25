#include <template/platformdefines.h>

#ifdef __WINDOWS__

#include "input/controller.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;
using namespace oi;

#include <Xinput.h>

void setVibration(u32 id, Vec2 amount) {
	amount = amount.clamp(0, 1);

	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	vibration.wLeftMotorSpeed = (WORD)(amount.x * 65535); // use any value between 0-65535 here
	vibration.wRightMotorSpeed = (WORD)(amount.y * 65535); // use any value between 0-65535 here

	XInputSetState(id, &vibration);
}

Controller::~Controller() {
	if(connected)
		setVibration(getId(), (flp) 0);
}

void Controller::update(Window *w, flp dt) {

	prev = next;
	prevConnected = connected;

	WindowInterface *wi = w->getInterface();

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	if (vibrationTime > 0) {
		vibrationTime -= dt;
		if (vibrationTime <= 0) {
			vibrationTime = 0;
			setVibration(getId(), (flp)0);
		}
	}

	if (connected = XInputGetState(getId(), &state) == ERROR_SUCCESS) {

		axes[ControllerAxis::Lx - 1U] = state.Gamepad.sThumbLX / 32767.f;
		axes[ControllerAxis::Ly - 1U] = state.Gamepad.sThumbLY / 32767.f;

		axes[ControllerAxis::Rx - 1U] = state.Gamepad.sThumbRX / 32767.f;
		axes[ControllerAxis::Ry - 1U] = state.Gamepad.sThumbRY / 32767.f;

		axes[ControllerAxis::L2 - 1U] = state.Gamepad.bLeftTrigger / 255.f;
		axes[ControllerAxis::R2 - 1U] = state.Gamepad.bRightTrigger / 255.f;

		next[ControllerButton::Cross - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
		next[ControllerButton::Square - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
		next[ControllerButton::Triangle - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
		next[ControllerButton::Circle - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;

		next[ControllerButton::Down - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
		next[ControllerButton::Left - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
		next[ControllerButton::Up - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
		next[ControllerButton::Right - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

		next[ControllerButton::L1 - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		next[ControllerButton::R1 - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		next[ControllerButton::L3 - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
		next[ControllerButton::R3 - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;

		next[ControllerButton::Options - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
		next[ControllerButton::Share - 1U] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;

		Controller *controller = w->getInputHandler().getController(getId());

		if(wi != nullptr)
			for (u32 i = 1U; i < ControllerButton::length; ++i) {

				Binding b = Binding(ControllerButton(i), getId());
				InputState state = getState(b);

				if (state == InputState::PRESSED)
					wi->onInput(controller, b, true);

				else if (state == InputState::RELEASED)
					wi->onInput(controller, b, false);
			}


	} else {
		memset(&axes, 0, sizeof(axes));
		next.reset();
	}

	if (wi != nullptr)
		if (prevConnected && !connected)
			wi->onDisconnect(this);
		else if(!prevConnected && connected)
			wi->onConnect(this);
}

void Controller::vibrate(Vec2 amount, f32 time) {
	setVibration(getId(), amount);
	vibrationTime = time;
}

#endif