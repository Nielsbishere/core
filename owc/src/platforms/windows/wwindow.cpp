#include <template/platformdefines.h>

#ifdef __WINDOWS__

#include <template/templatefuncs.h>
#include "window/windowinterface.h"
#include "window/windowmanager.h"
#include "input/mouse.h"
#include "input/keyboard.h"
using namespace oi::wc;
using namespace oi;

struct WWindowData {
	HWND wnd;
	HINSTANCE instance;
};

DEnum(WKey, u32, Undefined = 0, Zero = 48, One = 49, Two = 50, Three = 51, Four = 52, Five = 53, Six = 54, Seven = 55, Eight = 56, Nine = 57, A = 65, B = 66, C = 67, D = 68, E = 69,
	F = 70, G = 71, H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78, O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85, V = 86, W = 87, X = 88,
	Y = 89, Z = 90, Minus = VK_OEM_MINUS, Equals = VK_OEM_NEC_EQUAL, Left_brace = VK_OEM_4, Right_brace = VK_OEM_6, Semicolon = VK_OEM_1, Apostrophe = VK_OEM_7,
	Tilde = VK_OEM_3, Backslash = VK_OEM_5, Comma = VK_OEM_COMMA, Period = VK_OEM_PERIOD, Slash = VK_OEM_2, Space = VK_SPACE, n0 = VK_NUMPAD0, n1 = VK_NUMPAD1,
	n2 = VK_NUMPAD2, n3 = VK_NUMPAD3, n4 = VK_NUMPAD4, n5 = VK_NUMPAD5, n6 = VK_NUMPAD6, n7 = VK_NUMPAD7, n8 = VK_NUMPAD8, n9 = VK_NUMPAD9, Subtract = VK_SUBTRACT,
	Add = VK_ADD, Decimal = VK_DECIMAL, Multiply = VK_MULTIPLY, Divide = VK_DIVIDE, F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4, F5 = VK_F5, F6 = VK_F6, F7 = VK_F7,
	F8 = VK_F8, F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12, F13 = VK_F13, F14 = VK_F14, F15 = VK_F15, F16 = VK_F16, F17 = VK_F17, F18 = VK_F18, F19 = VK_F19,
	F20 = VK_F20, F21 = VK_F21, F22 = VK_F22, F23 = VK_F23, F24 = VK_F24, Up = VK_UP, Down = VK_DOWN, Left = VK_LEFT, Right = VK_RIGHT, Page_up = VK_NEXT, Page_down = VK_PRIOR,
	Home = VK_HOME, End = VK_END, Insert = VK_INSERT, Delete = VK_DELETE, Scroll_lock = VK_SCROLL, Num_lock = VK_NUMLOCK, Caps_lock = VK_CAPITAL,
	Tab = VK_TAB, Enter = VK_RETURN, Backspace = VK_BACK, Esc = VK_ESCAPE,
	Plus = VK_OEM_PLUS, Left_shift = VK_SHIFT, Right_shift = VK_SHIFT, Left_ctrl = VK_CONTROL, Right_ctrl = VK_CONTROL
);

namespace oi {

	namespace wc {

		struct Window_imp {

			static Window *getByHandle(HWND hwnd);
			static LRESULT CALLBACK windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

		};

	}

}

Window *Window_imp::getByHandle(HWND hwnd) {

	WindowManager *wm = WindowManager::get();

	for (u32 i = 0; i < wm->getWindows(); ++i) {

		Window *w = wm->operator[](i);
		WWindowData &dat = *(WWindowData*) w->platformData;

		if (dat.wnd == hwnd)
			return w;

	}

	return nullptr;
}

LRESULT CALLBACK Window_imp::windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;

	Window *w = getByHandle(hwnd);
	if (w == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);

	WindowInterface *wi = w->getInterface();
	WindowInfo &win = w->getInfo();

	WORD xid = GET_XBUTTON_WPARAM(wParam);

	switch (message) {

	case WM_KEYUP:
	case WM_KEYDOWN:
	{
		Binding b = Binding(Key(WKey::find((u32) wParam).getName()));
		Keyboard *keyboard = w->getInputHandler().getKeyboard();
		bool isDown = message == WM_KEYDOWN;

		if (wi != nullptr && b.getCode() != 0 && (!isDown || (isDown && keyboard->isUp(b))))
			wi->onInput(keyboard, b, isDown);

		if (b.getCode() != 0)
			keyboard->update(b, isDown);
	}
	break;

	case WM_PAINT:

		if (wi != nullptr)
			wi->repaint();

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_CLOSE:
		w->getParent()->remove(w);
		break;

	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		Vec2u size = Vec2u(rect.right - rect.left, rect.bottom - rect.top);

		win.size = size;

		if (wi != nullptr)
			wi->onResize(size);
	}
	break;

	case WM_MOVE:
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);

		Vec2i pos = Vec2i(rect.left, rect.top);

		win.position = pos;

		if (wi != nullptr)
			wi->onMove(pos);
	}
	break;

	case WM_SETFOCUS:

		win.inFocus = true;

		if (wi != nullptr)
			wi->setFocus(true);

		break;

	case WM_KILLFOCUS:

		win.inFocus = false;

		if (wi != nullptr)
			wi->setFocus(false);

		break;

	case WM_MOUSEMOVE:
		{
			Vec2 c = Vec2((flp)LOWORD(lParam), (flp)HIWORD(lParam)) / Vec2(w->getInfo().getSize());
			Mouse *mouse = w->getInputHandler().getMouse();
			mouse->axes[MouseAxis::X - 1] = c.x;
			mouse->axes[MouseAxis::Y - 1] = c.y;

			if (wi != nullptr)
				wi->onMouseMove(c);

		}
		break;

	case WM_MOUSEWHEEL:
		{
			flp perc = (flp) GET_WHEEL_DELTA_WPARAM(wParam) / (flp) WHEEL_DELTA;
			Mouse *mouse = w->getInputHandler().getMouse();
			mouse->axes[MouseAxis::Mouse_wheel - 1] = perc;

			if (wi != nullptr)
				wi->onMouseWheel(perc);

		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
	case WM_XBUTTONDOWN:
		{
			u32 val = ((u32) message - (u32) WM_LBUTTONDOWN);
			u32 dsp = val / 3U;
			bool isDown = val % 3U == 0U;

			MouseButton button;
			if (dsp == 0) button = MouseButton::Left;
			else if (dsp == 1) button = MouseButton::Right;
			else if (dsp == 2) button = MouseButton::Middle;
			else {

				if (xid == XBUTTON1) button = MouseButton::Back;
				else button = MouseButton::Forward;
				
				isDown = message == WM_XBUTTONDOWN;
			}

			Mouse *mouse = w->getInputHandler().getMouse();
			Binding b = button.getValue();

			mouse->update(b, isDown);

			if (wi != nullptr)
				wi->onInput(mouse, b, isDown);

		}

		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return (LRESULT) NULL;
}

void Window::initPlatform() {

	WWindowData &dat = *(WWindowData*) platformData;

	dat.instance = GetModuleHandle(NULL);

	String str = info.getTitle();

	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Window_imp::windowEvents;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = dat.instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = str.toCString();
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
		Log::throwError<Window, 0x0>("Couldn't init Windows class");

	int nStyle = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;

	u32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
	u32 screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (info.getSize() == 0)
		info.size = Vec2u(screenWidth, screenHeight);

	dat.wnd = CreateWindowEx(WS_EX_APPWINDOW, str.toCString(), str.toCString(), nStyle, info.getPosition().x, info.getPosition().y, info.getSize().x, info.getSize().y, NULL, NULL, dat.instance, NULL);

	if (dat.wnd == NULL)
		Log::throwError<Window, 0x1>("Couldn't init Windows window");

	RECT rect;
	GetClientRect(dat.wnd, &rect);

	i32 width = (i32)(rect.right - rect.left);
	i32 height = (i32)(rect.bottom - rect.top);

	i32 difX = (i32) info.getSize().x - width;
	i32 difY = (i32) info.getSize().y - height;

	SetWindowPos(dat.wnd, NULL, -difX / 2, 0, info.getSize().x + difX, info.getSize().y + difY, SWP_NOZORDER);

	info.focus();
	updatePlatform();
}

void Window::destroyPlatform() {

	WWindowData &dat = *(WWindowData*)platformData;

	if (dat.wnd != NULL) {
		PostQuitMessage(0);
		DestroyWindow(dat.wnd);
		dat.wnd = NULL;
	}
}

void Window::updatePlatform() {

	WWindowData &dat = *(WWindowData*)platformData;

	if (isSet(info.pending, WindowAction::MOVE) || isSet(info.pending, WindowAction::RESIZE))
		MoveWindow(dat.wnd, info.getPosition().x, info.getPosition().y, info.getSize().x, info.getSize().y, false);

	if (isSet(info.pending, WindowAction::IN_FOCUS))
		if (info.isInFocus()) {
			ShowWindow(dat.wnd, SW_SHOW);
			SetForegroundWindow(dat.wnd);
			SetFocus(dat.wnd);
		}
}

void Window::swapBuffers() {
	SwapBuffers(GetDC(((WWindowData*) platformData)->wnd));
}

#endif