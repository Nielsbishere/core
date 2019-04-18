#include <android/input.h>
#include "android/android.h"
#include "window/windowinterface.h"
#include "window/window.h"
#include "viewport/windowviewport.hpp"

#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/controller.h"

using namespace oi::wc;
using namespace oi;

WindowViewport::WindowViewport(const WindowInfo &info) : Viewport({ ViewportLayer() }), info(info) {
	ext = new WindowViewportExt();
}

void WindowViewport::init(Window *window) {

	w = window;

	ext->app = (android_app*)info.getApp();
	ext->app->userData = this;
	ext->app->onAppCmd = WindowViewportExt::handleCmd;
	ext->app->onInputEvent = WindowViewportExt::handleInput;
}

WindowViewport::~WindowViewport() {
	delete ext;
}

void WindowViewportExt::handleCmd(struct android_app *app, int32_t cmd) {

	Window *w = Window::get();
	WindowInterface *wi = w->getInterface();
	WindowViewport *wv = w->getViewport<WindowViewport>();

	switch (cmd) {

		case APP_CMD_GAINED_FOCUS:

			wv->getInfo().inFocus = true;

			if (wi)
				wi->setFocus(true);

			break;

		case APP_CMD_LOST_FOCUS:

			wv->getInfo().inFocus = false;

			if (wi)
				wi->setFocus(false);

			break;

		case APP_CMD_TERM_WINDOW:

			if (wi) {
				wi->destroySurface();
				wv->visible = false;
			}

			break;

		case APP_CMD_DESTROY:

			delete w;
			break;

		case APP_CMD_SAVE_STATE:

			if (wi) {

				app->savedState = std::malloc(1);
				app->savedStateSize = 1;

				wi->save("out/lifetime.bin");
			}

			break;

		case APP_CMD_INIT_WINDOW:

			if (wi) {

				if(app->savedState)
					wi->load("out/lifetime.bin");

				if (!wv->hasInitialized) {
					wi->init();
					wv->hasInitialized = true;
				}

				wv->visible = true;
				wv->getLayer(0).size = {};
			}

		case APP_CMD_CONTENT_RECT_CHANGED: {

			ARect rect = app->contentRect;

			Vec2i pos = Vec2i((i32)rect.left, (i32)rect.top);
			Vec2i oldPos = wv->getInfo().position;

			Vec2u size = Vec2u((u32)ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
			Vec2u oldSize = wv->getLayer(0).size;

			wv->getInfo().position = pos;
			wv->getLayer(0).size = size;

			if (wi && pos != oldPos)
				wi->onMove(pos);

			if (!wi || size == oldSize)
				break;

		}

		case APP_CMD_CONFIG_CHANGED:

			if (wi && wv->visible) {
				wi->onResize(wv->getLayer(0).size);

				Vec2 size = Vec2(wv->getLayer(0).size);

				if (size.x > size.y)
					size = size.swap();

				wi->onAspectChange(size.getAspect());
			}

			break;

		default:
			break;

	}

}

int32_t WindowViewportExt::handleInput(struct android_app *app, AInputEvent *ievent) {

	Window *w = Window::get();
	WindowInterface *wi = w->getInterface();
	WindowViewport *wv = w->getViewport<WindowViewport>();

	if (!wi) return 0;

	Keyboard *keyboard = w->getInputHandler().getKeyboard();

	bool isDown = AKeyEvent_getAction(ievent) == AKEY_EVENT_ACTION_DOWN;

	int32_t type = AInputEvent_getType(ievent);
	int32_t source = AInputEvent_getSource(ievent);

	switch (type) {

	case AINPUT_EVENT_TYPE_MOTION:
	{
		int32_t action = AMotionEvent_getAction(ievent);
		float x = AMotionEvent_getX(ievent, 0), y = AMotionEvent_getY(ievent, 0);

		if (source & AINPUT_SOURCE_JOYSTICK) {		//Update controller (PS4 implementation)

			float ax = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_X, 0);						//Left x
			float ay = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_Y, 0);						//Left y
			float az = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_Z, 0);						//Right x
			float arx = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_RX, 0) * 0.5 + 0.5;			//Left trigger
			float ary = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_RY, 0) * 0.5 + 0.5;			//Right trigger
			float arz = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_RZ, 0);						//Right y

			Controller *c = w->getInputHandler().getController(0);

			if (c != nullptr) {
				c->axes[ControllerAxis::Lx] = ax;
				c->axes[ControllerAxis::Ly] = ay;
				c->axes[ControllerAxis::Rx] = az;
				c->axes[ControllerAxis::Ry] = arz;
				c->axes[ControllerAxis::L2] = arx;
				c->axes[ControllerAxis::R2] = ary;
			}

		}
		else if (source & AINPUT_SOURCE_TOUCHSCREEN || source & AINPUT_SOURCE_MOUSE) {

			Vec2 pos = Vec2(x, y) / Vec2(wv->getLayer(0).size);
			pos.y = 1 - pos.y;

			Log::println(pos);

			Mouse *mouse = w->getInputHandler().getMouse();

			if (action == AMOTION_EVENT_ACTION_MOVE) {

				float xoff = AMotionEvent_getXOffset(ievent), yoff = AMotionEvent_getYOffset(ievent);
				if (wi != nullptr) wi->onMouseDrag(Vec2(xoff, yoff) / Vec2(wv->getLayer(0).size));

			}
			else if (action == AMOTION_EVENT_ACTION_POINTER_UP || AMOTION_EVENT_ACTION_UP) {

				Binding b = MouseButton::Left;
				mouse->update(b, isDown);

				if (isDown)
					mouse->axes[Mouse::prevAxes] = u16_MAX;

				if (wi != nullptr && mouse->prev[b.getCode() - 1] != isDown)
					wi->onInput(mouse, b, isDown);
			}

			mouse->axes[0] = pos.x;
			mouse->axes[1] = pos.y;
			mouse->axes[2] = AMotionEvent_getAxisValue(ievent, AMOTION_EVENT_AXIS_VSCROLL, 0);

		}
		else
			Log::warn(String("Motion ievent not supported; ") + source + " " + action + " " + x + " " + y);
	}

	return 0;

	case AINPUT_EVENT_TYPE_KEY:
	{
		int32_t keycode = AKeyEvent_getKeyCode(ievent);
		KeyExt key = KeyExt::find(keycode);
		Binding b = Key(key.getName()).getValue();

		if (key.getIndex() == 0) {

			ControllerButtonExt button = ControllerButtonExt::find(keycode);
			b = Binding(ControllerButton(button.getName()).getValue(), 0);

			if (button.getIndex() != 0) {

				Controller *c = w->getInputHandler().getController(0);
				c->update(b, isDown);

				if (wi != nullptr && c->prev[b.getCode() - 1] != isDown) wi->onInput(c, b, isDown);
			}

		}
		else {

			keyboard->update(b, isDown);

			if (wi != nullptr && keyboard->prev[b.getCode() - 1] != isDown) wi->onInput(keyboard, b, isDown);
		}
	}

	return 1;

	default:
		return 0;
	}
}

void WindowViewport::setInterface(WindowInterface *wif) {
	wi = wif;
	hasInitialized = false;
}

f32 WindowViewport::update() {

	int ident, events;
	struct android_poll_source *source;

	while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
		if (source)
			source->process(ext->app, source);
	}

	f32 dt = updateInternal();

	if (ext->app->destroyRequested)
		return f32_MAX;

	return dt;

}