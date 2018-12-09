#ifdef __ANDROID__

#include "platforms/generic.h"
#include "window/windowinterface.h"
#include "window/windowmanager.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/controller.h"

#include <android/input.h>
#include <cstdlib>

using namespace oi::wc;
using namespace oi;

Window *AWindow::getWindow(struct android_app *app){

	WindowManager *wm = WindowManager::get();

	for (u32 i = 0; i < wm->getWindows(); ++i) {

		Window *w = wm->operator[](i);

		if (w->getExtension().app == app)
			return w;

	}

	return nullptr;

}

void AWindow::initDisplay(Window *w){
	w->updatePlatform();
	w->initialized = true;
	w->finalize();
}

void AWindow::terminate(Window *w){
	WindowInterface *wi = w->getInterface();
	if(wi != nullptr)
		wi->destroySurface();

	w->initialized = false;
}

void AWindow::handleCmd(struct android_app *app, int32_t cmd){

	Window *w = getWindow(app);
	WindowInterface *wi = w->getInterface();

	if(w->initialized || cmd == APP_CMD_INIT_WINDOW)
		switch (cmd) {

		case APP_CMD_WINDOW_RESIZED:

			w->getInfo().size = Vec2u((u32)ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));

			if (wi != nullptr) {
				wi->onResize(w->getInfo().size);
				w->updateAspect();
			}

		break;

		case APP_CMD_CONTENT_RECT_CHANGED:
		{
			ARect rect = app->contentRect;

			Vec2i pos = Vec2i((i32)rect.left, (i32)rect.top);

			w->getInfo().position = pos;

			if (wi != nullptr)
				wi->onMove(pos);

		}
		break;

		case APP_CMD_GAINED_FOCUS:

			w->getInfo().inFocus = true;

			if (wi != nullptr)
				wi->setFocus(true);

			break;

		case APP_CMD_LOST_FOCUS:

			w->getInfo().inFocus = false;

			if (wi != nullptr)
				wi->setFocus(false);

			break;

		case APP_CMD_RESUME:
			Log::println("Resume");
			break;

		case APP_CMD_TERM_WINDOW:
			terminate(w);
			break;
		
		case APP_CMD_SAVE_STATE:
	
			//Set saved state
			//However, we use lifetime.bin as a save file
		
			app->savedState = std::malloc(1);
			app->savedStateSize = 1;
		
			if(wi != nullptr)
				wi->save("out/lifetime.bin");
		
			break;
		
		case APP_CMD_INIT_WINDOW:
		
			//Load saved state
			//However, we use lifetime.bin as a save file
		
			if(wi != nullptr && app->savedState != nullptr)
				wi->load("out/lifetime.bin");

			w->getInfo().size = Vec2u((u32)ANativeWindow_getWidth(app->window), ANativeWindow_getHeight(app->window));
			initDisplay(w);
			w->initialized = false;
			w->updateAspect();
			w->initialized = true;

			break;

		case APP_CMD_CONFIG_CHANGED:

			w->updateAspect();
			break;

		default:
			break;

		}

}

int32_t AWindow::handleInput(struct android_app *app, AInputEvent *event){

	Window *w = getWindow(app);
	WindowInterface *wi = w->getInterface();

	if(!w->initialized) return 0;
	
	Keyboard *keyboard = w->getInputHandler().getKeyboard();

	bool isDown = AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN;
	
	int32_t type = AInputEvent_getType(event);
	int32_t source = AInputEvent_getSource(event);
	
	switch(type){

		case AINPUT_EVENT_TYPE_MOTION:
			{
				int32_t action = AMotionEvent_getAction(event);
				float x = AMotionEvent_getX(event, 0), y = AMotionEvent_getY(event, 0);
				
				if(source & AINPUT_SOURCE_JOYSTICK){		//Update controller (PS4 implementation)
				
					float ax = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);						//Left x
					float ay = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);						//Left y
					float az = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0);						//Right x
					float arx = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RX, 0) * 0.5 + 0.5;			//Left trigger
					float ary = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RY, 0) * 0.5 + 0.5;			//Right trigger
					float arz = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0);						//Right y
				
					Controller *c = w->getInputHandler().getController(0);
					
					if(c != nullptr){
						c->axes[ControllerAxis::Lx] = ax;
						c->axes[ControllerAxis::Ly] = ay;
						c->axes[ControllerAxis::Rx] = az;
						c->axes[ControllerAxis::Ry] = arz;
						c->axes[ControllerAxis::L2] = arx;
						c->axes[ControllerAxis::R2] = ary;
					}
					
				} else if(source & AINPUT_SOURCE_TOUCHSCREEN || source & AINPUT_SOURCE_MOUSE){ 
				
					Vec2 pos = Vec2(x, y) / Vec2(w->getInfo().size);
					pos.y = 1 - pos.y;
					
					Mouse *mouse = w->getInputHandler().getMouse();
					
					if(action == AMOTION_EVENT_ACTION_MOVE){
						
						float xoff = AMotionEvent_getXOffset(event), yoff = AMotionEvent_getYOffset(event);
						if(wi != nullptr) wi->onMouseDrag(Vec2(xoff, yoff) / Vec2(w->getInfo().size));
						
					} else if(action == AMOTION_EVENT_ACTION_POINTER_UP || AMOTION_EVENT_ACTION_UP){
						
						Binding b = MouseButton::Left;
						mouse->update(b, isDown);
						
						if(wi != nullptr && mouse->prev[b.getCode() - 1] != isDown)
							wi->onInput(mouse, b, isDown);
					}
				
					mouse->axes[0] = pos.x;
					mouse->axes[1] = pos.y;
					mouse->axes[2] = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);

				} else
					Log::warn(String("Motion event not supported; ") + source + " " + action + " " + x + " " + y);
			}
			return 0;

		case AINPUT_EVENT_TYPE_KEY:
			{
				int32_t keycode = AKeyEvent_getKeyCode(event);
				AKey key = AKey::find(keycode);
				Binding b = Key(key.getName()).getValue();
				
				if(key.getIndex() == 0){

					AControllerButton button = AControllerButton::find(keycode);
					b = Binding(ControllerButton(button.getName()).getValue(), 0);

					if(button.getIndex() != 0){
						
						Controller *c = w->getInputHandler().getController(0);
						c->update(b, isDown);
						
						if(wi != nullptr && c->prev[b.getCode() - 1] != isDown) wi->onInput(c, b, isDown);
					}

				} else {
					
					keyboard->update(b, isDown);
					
					if(wi != nullptr && keyboard->prev[b.getCode() - 1] != isDown) wi->onInput(keyboard, b, isDown);
				}
			}

			return 1;

		default:
			return 0;
	}
}


u32 Window::getSurfaceSize(){ return (u32) sizeof(ANativeWindow*); }

void *Window::getSurfaceData() {
	return (void*) &ext.app->window;
}

void Window::initPlatform() {

	//Get data

	ext.app = getInfo().handle;
	ext.app->userData = this;
	ext.app->onAppCmd = AWindow::handleCmd;
	ext.app->onInputEvent = AWindow::handleInput;
}

void Window::destroyPlatform() {
	AWindow::terminate(this);
}

void Window::updatePlatform() {

	if (isSet(info.pending, WindowAction::FULL_SCREEN))
		Log::warn("fullScreen action is handled by Android, not the application");

	if (isSet(info.pending, WindowAction::IN_FOCUS))
		Log::warn("setFocus action is not supported on Android");
}

#endif