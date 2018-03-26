#include <template/platformdefines.h>

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

using namespace oi::wc;
using namespace oi;

struct AWindowData {
	struct android_app *app;
	EGLDisplay display;
	EGLSurface surface;
	EGLConfig config;
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
	Volume_down = AKEYCODE_VOLUME_DOWN, Volume_up = AKEYCODE_VOLUME_UP, Power = AKEYCODE_POWER
);

DEnum(AControllerButton, int32_t, Undefined = AKEYCODE_UNKNOWN, Cross = AKEYCODE_BUTTON_A, Square = AKEYCODE_BUTTON_X, Triangle = AKEYCODE_BUTTON_Y, Circle = AKEYCODE_BUTTON_B, 
	Down = AKEYCODE_DPAD_DOWN, Left = AKEYCODE_DPAD_LEFT, Up = AKEYCODE_DPAD_UP, Right = AKEYCODE_DPAD_RIGHT, L1 = AKEYCODE_BUTTON_L1, R1 = AKEYCODE_BUTTON_R1, 
	L3 = AKEYCODE_BUTTON_THUMBL, R3 = AKEYCODE_BUTTON_THUMBR, Share = AKEYCODE_BUTTON_SELECT, Options = AKEYCODE_BUTTON_START
);

namespace oi {

	namespace wc {

		struct Window_imp {

			static Window *getWindow(struct android_app *app);
			static void handleCmd(struct android_app *app, int32_t cmd);
			static int32_t handleInput(struct android_app *app, AInputEvent *event);

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

void Window_imp::handleCmd(struct android_app *app, int32_t cmd){

	Window *w = getWindow(app);
	WindowInterface *wi = w->getInterface();

	switch(cmd){

	case APP_CMD_TERM_WINDOW:
		w->getParent()->remove(w);
		break;

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
		
		if (wi != nullptr)
			wi->setFocus(true);
		
		break;
	
	case APP_CMD_LOST_FOCUS:

		w->getInfo().inFocus = true;
		
		if (wi != nullptr)
			wi->setFocus(true);
		
		break;

	case APP_CMD_CONFIG_CHANGED:
		Log::warn("Config changed event not supported");	//TODO:
		break;

	case APP_CMD_RESUME:
		Log::warn("Resume event not supported");			//TODO:
		break;

	case APP_CMD_PAUSE:
		Log::warn("Pause event not supported");				//TODO:
		break;

	case APP_CMD_SAVE_STATE:
		Log::warn("Save state event not supported");		//TODO:
		break;

	default:
		break;

	}

}

int32_t Window_imp::handleInput(struct android_app *app, AInputEvent *event){

	Window *w = getWindow(app);
	Keyboard *keyboard = w->getInputHandler().getKeyboard();

	bool isDown = AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN;

	int32_t type = AInputEvent_getType(event);
	switch(type){

		case AINPUT_EVENT_TYPE_MOTION:
			{
				float x = AMotionEvent_getX(event, 0), y = AMotionEvent_getY(event, 0);
			}
			Log::warn("Motion event not supported");	//TODO:
			return 0;

		case AINPUT_EVENT_TYPE_KEY:
			{
				int32_t keycode = AKeyEvent_getKeyCode(event);
				AKey key = AKey::find(keycode);

				if(key.getIndex() == 0){

					AControllerButton button = AControllerButton::find(keycode);

					if(button.getIndex() == 0)
						w->getInputHandler().getController(0)->update(Binding(ControllerButton(button.getName()), 0), isDown);

				} else
					keyboard->update(Binding(Key(key.getName())), isDown);
			}

			return 1;

		default:
			return 0;
	}
}

void Window::initPlatform() {

	//Get data
	AWindowData &dat = *(AWindowData*) platformData;

	dat.app = * (struct android_app**) getInfo().handle;
	dat.app->userData = platformData;
	dat.app->onAppCmd = Window_imp::handleCmd;
	dat.app->onInputEvent = Window_imp::handleInput;

	//Define attributes (8 Bpc = 24Bpp with depth buffer)
	const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
			EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

	//Surface info (like size, etc.)
	EGLint width, height, format, numConfigs;
	
	//Get display
	EGLDisplay &display = dat.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);

	if(display == NULL)
		Log::throwError<Window, 0x0>("Couldn't find a display");

	//Get num configs
	eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

	if(numConfigs == 0)
		Log::throwError<Window, 0x1>("Couldn't find any config");

	//Get configs
	EGLConfig *cfg = new EGLConfig[numConfigs];

	eglChooseConfig(display, attribs, cfg, numConfigs, &numConfigs);

	//Get valid config
	EGLConfig &config = dat.config;
	bool foundConfig = false;

	for(u32 i = 0; i < numConfigs; ++i){

		EGLConfig &conf = cfg[i];
		bool hasTags = true;

		//Check if supported
		EGLint *nextAttrib = (EGLint*) attribs;
		while(*nextAttrib != EGL_NONE){

			EGLint val;
			if(!eglGetConfigAttrib(display, conf, *nextAttrib, &val) || val != *(nextAttrib + 1)){
				hasTags = false;
				break;
			}

			nextAttrib += 2;
		}

		if(hasTags){
			config = conf;
			foundConfig = true;
			break;
		}
		
	}

	delete[] cfg;

	if(!foundConfig)
		Log::throwError<Window, 0x2>("Couldn't find a suitable config");

	//Create surface
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	EGLSurface &surface = dat.surface = eglCreateWindowSurface(display, config, dat.app->window, NULL);

	eglQuerySurface(display, surface, EGL_WIDTH, &width);
	eglQuerySurface(display, surface, EGL_HEIGHT, &height);

	info.size = Vec2u((u32) width, (u32) height);
	
	//Update
	updatePlatform();
}

void Window::destroyPlatform() {

	AWindowData &dat = *(AWindowData*)platformData;

    eglMakeCurrent(dat.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (dat.surface != EGL_NO_SURFACE)
        eglDestroySurface(dat.display, dat.surface);

    eglTerminate(dat.display);

}

void Window::updatePlatform() {

	AWindowData &dat = *(AWindowData*)platformData;

	if (isSet(info.pending, WindowAction::MOVE) || isSet(info.pending, WindowAction::RESIZE))
		Log::warn("resize/move action is not supported on Android");

	if (isSet(info.pending, WindowAction::IN_FOCUS))
		Log::warn("setFocus action is not supported on Android");
}

void Window::swapBuffers() {
	AWindowData &dat = *(AWindowData*)platformData;
	eglSwapBuffers(dat.display, dat.surface);
}

#endif