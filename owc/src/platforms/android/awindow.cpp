#ifdef __ANDROID__

#include <template/templatefuncs.h>
#include "window/windowinterface.h"
#include "window/windowmanager.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/controller.h"

#include <EGL/egl.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include <cstdlib>

using namespace oi::wc;
using namespace oi;

struct AWindowData {
	struct android_app *app;
	EGLDisplay display;
	EGLSurface surface;
	EGLConfig config;
	EGLContext context;
};

DEnum(AKey, int32_t, Undefined = AKEYCODE_UNKNOWN, Zero = AKEYCODE_0, One = AKEYCODE_1, Two = AKEYCODE_2, Three = AKEYCODE_3, Four = AKEYCODE_4, Five = AKEYCODE_5, Six = AKEYCODE_6, Seven = AKEYCODE_7, Eight = AKEYCODE_8, Nine = AKEYCODE_9, 
	A = AKEYCODE_A, B = AKEYCODE_B, C = AKEYCODE_C, D = AKEYCODE_D, E = AKEYCODE_E, F = AKEYCODE_F, G = AKEYCODE_G, H = AKEYCODE_H, I = AKEYCODE_I, J = AKEYCODE_J, K = AKEYCODE_K, L = AKEYCODE_L, M = AKEYCODE_M, N = AKEYCODE_N, O = AKEYCODE_O, 
	P = AKEYCODE_P, Q = AKEYCODE_Q, R = AKEYCODE_R, S = AKEYCODE_S, T = AKEYCODE_T, U = AKEYCODE_U, V = AKEYCODE_V, W = AKEYCODE_W, X = AKEYCODE_X, Y = AKEYCODE_Y, Z = AKEYCODE_Z, Minus = AKEYCODE_MINUS, Equals = AKEYCODE_EQUALS, Left_brace = AKEYCODE_LEFT_BRACKET, 
	Right_brace = AKEYCODE_RIGHT_BRACKET, Semicolon = AKEYCODE_SEMICOLON, Apostrophe = AKEYCODE_APOSTROPHE, Tilde = AKEYCODE_GRAVE, Backslash = AKEYCODE_BACKSLASH, Comma = AKEYCODE_COMMA, Period = AKEYCODE_PERIOD, Slash = AKEYCODE_SLASH, Space = AKEYCODE_SPACE, 
	n0 = AKEYCODE_NUMPAD_0, n1 = AKEYCODE_NUMPAD_1, n2 = AKEYCODE_NUMPAD_2, n3 = AKEYCODE_NUMPAD_3, n4 = AKEYCODE_NUMPAD_4, n5 = AKEYCODE_NUMPAD_5, n6 = AKEYCODE_NUMPAD_6, n7 = AKEYCODE_NUMPAD_7, n8 = AKEYCODE_NUMPAD_8, n9 = AKEYCODE_NUMPAD_9, 
	Subtract = AKEYCODE_NUMPAD_SUBTRACT, Add = AKEYCODE_NUMPAD_ADD, Decimal = AKEYCODE_NUMPAD_DOT, Multiply = AKEYCODE_NUMPAD_MULTIPLY, Divide = AKEYCODE_NUMPAD_DIVIDE, F1 = AKEYCODE_F1, F2 = AKEYCODE_F2, F3 = AKEYCODE_F3, F4 = AKEYCODE_F4, 
	F5 = AKEYCODE_F5, F6 = AKEYCODE_F6, F7 = AKEYCODE_F7, F8 = AKEYCODE_F8, F9 = AKEYCODE_F9, F10 = AKEYCODE_F10, F11 = AKEYCODE_F11, F12 = AKEYCODE_F12, Page_up = AKEYCODE_PAGE_UP, Page_down = AKEYCODE_PAGE_DOWN,
	Home = AKEYCODE_HOME, Insert = AKEYCODE_INSERT, Scroll_lock = AKEYCODE_SCROLL_LOCK, Num_lock = AKEYCODE_NUM_LOCK, Caps_lock = AKEYCODE_CAPS_LOCK, Tab = AKEYCODE_TAB, Enter = AKEYCODE_ENTER, Backspace = AKEYCODE_DEL, Esc = AKEYCODE_ESCAPE,
	Plus = AKEYCODE_PLUS, Left_shift = AKEYCODE_SHIFT_LEFT, Right_shift = AKEYCODE_SHIFT_RIGHT, Left_alt = AKEYCODE_ALT_LEFT, Right_alt = AKEYCODE_ALT_RIGHT,
	Volume_down = AKEYCODE_VOLUME_DOWN, Volume_up = AKEYCODE_VOLUME_UP, Power = AKEYCODE_POWER, Back = AKEYCODE_BACK, Delete = AKEYCODE_FORWARD_DEL,
	Up = AKEYCODE_DPAD_UP, Down = AKEYCODE_DPAD_DOWN, Left = AKEYCODE_DPAD_LEFT, Right = AKEYCODE_DPAD_RIGHT
);

//PS4 implementation
DEnum(AControllerButton, int32_t, Undefined = AKEYCODE_UNKNOWN, Cross = AKEYCODE_BUTTON_B, Square = AKEYCODE_BUTTON_A, Triangle = AKEYCODE_BUTTON_X, Circle = AKEYCODE_BUTTON_C, 
	L1 = AKEYCODE_BUTTON_Y, R1 = AKEYCODE_BUTTON_Z, L3 = AKEYCODE_BUTTON_SELECT, R3 = AKEYCODE_BUTTON_START, Share = AKEYCODE_BUTTON_L2, Options = AKEYCODE_BUTTON_R2
);

namespace oi {

	namespace wc {

		struct Window_imp {

			static Window *getWindow(struct android_app *app);
			static void handleCmd(struct android_app *app, int32_t cmd);
			static int32_t handleInput(struct android_app *app, AInputEvent *event);
			static void initDisplay(Window *w);
			static void terminate(Window *w);
			
		};

	}

}

Window *Window_imp::getWindow(struct android_app *app){

	WindowManager *wm = WindowManager::get();

	for (u32 i = 0; i < wm->getWindows(); ++i) {

		Window *w = wm->operator[](i);
		AWindowData &dat = *(AWindowData*) w->platformData;

		if (dat.app == app)
			return w;

	}

	return nullptr;

}

void Window_imp::initDisplay(Window *w){
	w->updatePlatform();
	w->initialized = true;
	w->finalize();
}

void Window_imp::terminate(Window *w){
	WindowInterface *wi = w->getInterface();
	if(wi != nullptr)
		wi->destroySurface();
}

void Window_imp::handleCmd(struct android_app *app, int32_t cmd){

	Window *w = getWindow(app);
	WindowInterface *wi = w->getInterface();

	if(w->initialized || cmd == APP_CMD_INIT_WINDOW)
		switch (cmd) {

		case APP_CMD_WINDOW_RESIZED:

		{
			int32_t width = ANativeWindow_getWidth(app->window);
			int32_t height = ANativeWindow_getHeight(app->window);

			w->getInfo().size = Vec2u((u32)width, (u32)height);
		}

		if (wi != nullptr)
			wi->onResize(w->getInfo().size);


		break;

		case APP_CMD_CONTENT_RECT_CHANGED:
		{
			ARect rect = app->contentRect;

			Vec2i pos = Vec2i((i32)rect.left, (i32)rect.top);

			Log::println("Resize?");

			w->getInfo().position = pos;

			if (wi != nullptr)
				wi->onMove(pos);
		}
		break;

		case APP_CMD_WINDOW_REDRAW_NEEDED:

			if (wi != nullptr)
				wi->repaint();

			break;

		case APP_CMD_GAINED_FOCUS:

			w->getInfo().inFocus = true;
			w->isPaused = false;

			if (wi != nullptr)
				wi->setFocus(true);

			break;

		case APP_CMD_LOST_FOCUS:

			w->getInfo().inFocus = true;
			w->isPaused = true;

			if (wi != nullptr)
				wi->setFocus(true);

			break;

		case APP_CMD_RESUME:
			Log::println("Resume");
			break;

		case APP_CMD_TERM_WINDOW:
			if (!w->configChanged)
				terminate(w);
			else {
				int32_t width = ANativeWindow_getWidth(app->window);
				int32_t height = ANativeWindow_getHeight(app->window);

				Vec2u osize = w->getInfo().size;
				w->getInfo().size = Vec2u((u32)width, (u32)height);

				w->configChanged = false;

				if (wi != nullptr)
					wi->onResize(w->getInfo().size);
			}
		break;
		
	case APP_CMD_SAVE_STATE:
	
		//Set saved state
		//However, we use lifetime.bin as a save file
		
		app->savedState = std::malloc(1);
		app->savedStateSize = 1;
		
		if(wi != nullptr)
			wi->save("lifetime.bin");
		
		break;
		
	case APP_CMD_INIT_WINDOW:
		
		//Load saved state
		//However, we use lifetime.bin as a save file
		
		if(wi != nullptr && app->savedState != nullptr)
			wi->load("lifetime.bin");
		
		initDisplay(w);
		break;

	case APP_CMD_CONFIG_CHANGED:
		w->configChanged = true;
		break;

	default:
		break;

	}

}

int32_t Window_imp::handleInput(struct android_app *app, AInputEvent *event){

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
				
					mouse->axes[0] = x;
					mouse->axes[1] = y;
					mouse->axes[2] = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
					
				} else
					Log::warn(String("Motion event not supported; ") + source + " " + action + " " + " " + x + " " + y);
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
	AWindowData &dat = *(AWindowData*) platformData;
	return (void*) &dat.app->window;
}

void Window::initPlatform() {

	//Get data
	AWindowData &dat = *(AWindowData*) platformData;

	dat.app = (struct android_app*) getInfo().handle;
	dat.app->userData = platformData;
	dat.app->onAppCmd = Window_imp::handleCmd;
	dat.app->onInputEvent = Window_imp::handleInput;
}

void Window::destroyPlatform() {
	Window_imp::terminate(this);
}

void Window::updatePlatform() {

	AWindowData &dat = *(AWindowData*)platformData;

	if (isSet(info.pending, WindowAction::MOVE) || isSet(info.pending, WindowAction::RESIZE))
		Log::warn("resize/move action is not supported on Android");

	if (isSet(info.pending, WindowAction::IN_FOCUS))
		Log::warn("setFocus action is not supported on Android");
}

#endif