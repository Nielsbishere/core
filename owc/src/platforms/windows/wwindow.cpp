#ifdef __WINDOWS__

#include "platforms/generic.h"
#include "window/windowinterface.h"
#include "window/windowmanager.h"
#include "input/mouse.h"
#include "input/keyboard.h"
using namespace oi::wc;
using namespace oi;

Window *WWindow::getByHandle(HWND hwnd) {

	WindowManager *wm = WindowManager::get();

	for (u32 i = 0; i < wm->getWindows(); ++i) {
		Window *w = wm->operator[](i);

		if (w->getExtension().window == hwnd)
			return w;
	}

	return nullptr;
}

LRESULT CALLBACK WWindow::windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

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

		Vec2u prevSize = win.size;
		win.size = size;

		if (size.x == 0 || size.y == 0)
			w->pause();
		else {
			w->pause(false);
			if (wi != nullptr)
				wi->onResize(size);
		}
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
			
			Vec2 &ax = *(Vec2*) mouse->axes;
			Vec2 d = c - ax;
			
			ax = c;

			if (wi != nullptr){
				
				if(mouse->next[0])
					wi->onMouseDrag(ax);
				else
					wi->onMouseMove(c);
			}

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

	ext.instance = GetModuleHandle(NULL);

	String str = info.getTitle();

	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WWindow::windowEvents;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ext.instance;
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

	info.size = Vec2u(screenWidth, screenHeight);

	ext.window = CreateWindowEx(WS_EX_APPWINDOW, str.toCString(), str.toCString(), nStyle, info.getPosition().x, info.getPosition().y, info.getSize().x, info.getSize().y, NULL, NULL, ext.instance, NULL);

	if (ext.window == NULL)
		Log::throwError<Window, 0x1>("Couldn't init Windows window");

	info.focus();
	updatePlatform();
	
	initialized = true;
	finalize();
}

u32 Window::getSurfaceSize(){ return (u32) sizeof(WWindow); }
void *Window::getSurfaceData() { return &ext; }

void Window::destroyPlatform() {
	if (ext.window != NULL) {
		PostQuitMessage(0);
		DestroyWindow(ext.window);
		ext.window = NULL;
	}
}

void Window::updatePlatform() {

	if (isSet(info.pending, WindowAction::MOVE) || isSet(info.pending, WindowAction::RESIZE))
		MoveWindow(ext.window, info.getPosition().x, info.getPosition().y, info.getSize().x, info.getSize().y, false);

	if (isSet(info.pending, WindowAction::IN_FOCUS))
		if (info.isInFocus()) {
			ShowWindow(ext.window, SW_SHOW);
			SetForegroundWindow(ext.window);
			SetFocus(ext.window);
		}
}

#endif