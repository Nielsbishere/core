#include "Window/WindowHandle.h"
#include "Window/WindowInfo.h"
#include "Window/Window.h"
#include "Input/Binding.h"
using namespace oi::wc;
using namespace oi;

#ifdef __WINDOWS__

void WindowHandle::destroy() {

	if (window != NULL) {
		PostQuitMessage(0);
		DestroyWindow(window);
	}

	window = NULL;
}

LRESULT CALLBACK WindowHandle::windowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	HDC hdc;

	Window *w = Window::getByHandle({ hwnd, 0 });
	if (w == nullptr) return DefWindowProc(hwnd, message, wParam, lParam);

	WindowInterface *wi = w->getInterface();
	WindowInfo &win = w->getInfo();

	WORD xid = GET_XBUTTON_WPARAM(wParam);

	switch (message) {
	case WM_KEYUP:
		{
			Binding b = Binding(Key_s((u32)wParam));

			if (b.getCode() != 0){
				if (wi != nullptr)
					wi->onKeyRelease(b);

				w->handleBinding(b.toKey(), false);
			}
		}
		break;
	case WM_KEYDOWN:
		{
			Binding b = Binding(Key_s((u32)wParam));

			if (b.getCode() != 0) {
				if (wi != nullptr && !w->getInput().isDown(b))
					wi->onKeyPress(b);

				w->handleBinding(b.toKey(), true);
			}
		}
		break;
	case WM_PAINT:

		if (wi != nullptr)
			wi->repaint();

		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	case WM_CLOSE:
		w->destroy();
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
			GetClientRect(hwnd, &rect);

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

		win.moveCursor(Vec2u((u32)LOWORD(lParam), (u32)HIWORD(lParam)));

		if (wi != nullptr)
			wi->onMouseMove(w->getInfo().getCursor());

		break;

	case WM_LBUTTONDOWN:

		w->handleBinding(Click::Left, true);

		if (wi != nullptr)
			wi->onKeyPress(Click::Left);

		break;
	case WM_LBUTTONUP:

		w->handleBinding(Click::Left, false);

		if (wi != nullptr)
			wi->onKeyRelease(Click::Left);

		break;
	case WM_MBUTTONDOWN:

		w->handleBinding(Click::Middle, true);

		if (wi != nullptr)
			wi->onKeyPress(Click::Middle);

		break;
	case WM_MBUTTONUP:

		w->handleBinding(Click::Middle, false);

		if (wi != nullptr)
			wi->onKeyRelease(Click::Middle);

		break;
	case WM_RBUTTONDOWN:

		w->handleBinding(Click::Right, true);

		if (wi != nullptr)
			wi->onKeyPress(Click::Right);

		break;
	case WM_RBUTTONUP:

		w->handleBinding(Click::Right, false);

		if (wi != nullptr)
			wi->onKeyRelease(Click::Right);

		break;
	case WM_XBUTTONUP:

		if (xid == XBUTTON1) {
			w->handleBinding(Click::Back, false);

			if (wi != nullptr)
				wi->onKeyRelease(Click::Back);
		} else {
			w->handleBinding(Click::Forward, false);

			if (wi != nullptr)
				wi->onKeyRelease(Click::Forward);
		}

		break;
	case WM_XBUTTONDOWN:

		if (xid == XBUTTON1) {
			w->handleBinding(Click::Back, true);

			if (wi != nullptr)
				wi->onKeyPress(Click::Back);
		}
		else {
			w->handleBinding(Click::Forward, true);

			if (wi != nullptr)
				wi->onKeyPress(Click::Forward);
		}

		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return NULL;
}

bool WindowHandle::sameId(const WindowHandle &other) {
	return window == other.window;
}

bool WindowHandle::instantiate(WindowHandle &result, WindowInfo &info) {

	result.instance = GetModuleHandle(NULL);

	std::string str = info.getTitle();

	//Set up a basic window
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = windowEvents;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = result.instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = str.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
		return false;

	int nStyle = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;

	u32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
	u32 screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (info.getSize() == 0) 
		info.resize(Vec2u(screenWidth, screenHeight));

	result.window = CreateWindowEx(WS_EX_APPWINDOW, str.c_str(), str.c_str(), nStyle, info.getPosition().x(), info.getPosition().y(), info.getSize().x(), info.getSize().y(), NULL, NULL, result.instance, NULL);
	
	if (result.window == NULL) 
		return false;

	//Resize to actually become the desired size

	RECT rect;
	GetClientRect(result.window, &rect);

	i32 width = (i32)(rect.right - rect.left);
	i32 height = (i32)(rect.bottom - rect.top);

	i32 difX = (i32)info.getSize().x() - width;
	i32 difY = (i32)info.getSize().y() - height;

	SetWindowPos(result.window, NULL, -difX / 2, 0, info.getSize().x() + difX, info.getSize().y() + difY, SWP_NOZORDER);

	//Show window

	result.update(info, WindowAction::SHOW);

	return true;
}

bool isSet(WindowAction action, WindowAction target) {
	return ((u32)action & (u32)target) != 0;
}

void WindowHandle::update(WindowInfo &info, WindowAction action) {

	if (isSet(action, WindowAction::MOVE) || isSet(action, WindowAction::RESIZE))
		MoveWindow(window, info.getPosition().x(), info.getPosition().y(), info.getSize().x(), info.getSize().y(), false);

	if (isSet(action, WindowAction::IN_FOCUS))
		if (info.isInFocus()) {
			ShowWindow(window, SW_SHOW);
			SetForegroundWindow(window);
			SetFocus(window);
		}
}

void WindowHandle::swapBuffers() {
	SwapBuffers(GetDC(window));
}

#endif