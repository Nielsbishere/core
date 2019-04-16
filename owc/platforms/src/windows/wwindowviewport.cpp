#include "window/windowinterface.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "windows/windows.h"
#include "window/window.h"
#include "viewport/windowviewport.hpp"
using namespace oi::wc;
using namespace oi;

Window *WindowViewportExt::getByHandle(HWND hwnd) {

	if (Window *w = Window::get())
		if(WindowViewport *wv = Window::get()->getViewport<WindowViewport>())
			if(wv->getExt()->window == hwnd)
				return w;

	return nullptr;
}

LRESULT CALLBACK WindowViewportExt::windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;

	Window *w = getByHandle(hwnd);
	if (w == nullptr) return DefWindowProcA(hwnd, message, wParam, lParam);

	WindowViewport *wv = w->getViewport<WindowViewport>();
	if (wv == nullptr) return DefWindowProcA(hwnd, message, wParam, lParam);

	WindowInterface *wi = w->getInterface();
	WindowInfo &win = wv->getInfo();

	WORD xid = GET_XBUTTON_WPARAM(wParam);

	switch (message) {

	case WM_KEYUP:
	case WM_KEYDOWN:
	{
		Binding b = Binding(Key(KeyExt::find((u32) wParam).getName()));
		Keyboard *keyboard = w->getInputHandler().getKeyboard();
		bool isDown = message == WM_KEYDOWN;

		if (wi != nullptr && b.getCode() != 0 && (!isDown || keyboard->isUp(b)))
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
		delete w;
		break;

	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		Vec2u &size = wv->getLayer(0).size, prevSize = size;
		size = Vec2u(rect.right - rect.left, rect.bottom - rect.top);

		win.minimized = IsIconic(hwnd);

		if (size.x != 0 && size.y != 0 && wi != nullptr) {
			wi->onResize(size);
			wi->onAspectChange(Vec2(size).getAspect());
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
			Vec2 c = Vec2(LOWORD(lParam), HIWORD(lParam)) / Vec2(wv->getLayer(0).size);
			Mouse *mouse = w->getInputHandler().getMouse();
			
			c.y = -c.y;

			Vec2 &ax = *(Vec2*) mouse->axes;
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
			f32 perc = (f32) GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
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


WindowViewport::WindowViewport(const WindowInfo &info) : Viewport({ ViewportLayer() }), info(info) {
	ext = new WindowViewportExt();
}

WindowViewport::~WindowViewport() {

	if (ext->window) {
		PostQuitMessage(0);
		DestroyWindow(ext->window);
		ext->window = NULL;
	}

	delete ext;
}

void WindowViewport::init(Window *window) {

	w = window;

	ext->instance = GetModuleHandleA(NULL);

	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowViewportExt::windowEvents;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ext->instance;
	wc.hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 32, 32, 0);
	wc.hIconSm = (HICON)LoadImageA(GetModuleHandleA(NULL), "LOGO", IMAGE_ICON, 16, 16, 0);
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = w->getTitle().begin();
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassExA(&wc)) {
		HRESULT res = GetLastError();
		Log::error(res);
		Log::throwError<WindowViewportExt, 0x0>("Couldn't init Windows class");
	}

	int nStyle = WS_VISIBLE;

	if (info.isDecorated())
		nStyle |= WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;
	else
		nStyle |= WS_POPUP;

	u32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
	u32 screenHeight = GetSystemMetrics(SM_CYSCREEN);

	ext->window = CreateWindowExA(WS_EX_APPWINDOW, w->getTitle().begin(), w->getTitle().begin(), nStyle, info.getPosition().x, info.getPosition().y, screenWidth, screenHeight, NULL, NULL, ext->instance, NULL);

	if (ext->window == NULL)
		Log::throwError<WindowViewportExt, 0x1>("Couldn't init Windows window");

	RECT rect;
	GetClientRect(ext->window, &rect);

	Vec2u size((u32)(rect.right - rect.left), (u32)(rect.bottom - rect.top));
	getLayer(0).size = size;

	ShowWindow(ext->window, SW_SHOW);
	SetFocus(ext->window);

}

f32 WindowViewport::update() {

	MSG msg;

	while (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE)) {

		TranslateMessage(&msg);
		DispatchMessageA(&msg);

		if (msg.message == WM_CLOSE)
			return f32_MAX;

	}

	return updateInternal();
}