#include "Window/Window.h"
#include <Utils/Log.h>
using namespace oi::wc;
using namespace oi;

Window Window::windows[maxWindows];

Window::Window() : activated(false), index(0), info(WindowInfo("NULL", { 0, 0 }, { 0, 0 }, false)), wi(nullptr) { }

Window &Window::create(WindowInfo info) {

	Window *wptr = nullptr;

	for (u32 i = 0; i < maxWindows; ++i)
		if (!windows[i].activated){
			wptr = windows + i;
			wptr->index = i;
			break;
		}

	if (wptr == nullptr)
		Log::throwError<Window, 0x0>("Reached maximum amount of windows, don't forget to destroy windows when you're done with them.");

	info.initParent(wptr);
	wptr->info = info;

	if(!WindowHandle::instantiate(wptr->handle, info))
		Log::throwError<Window, 0x1>("Couldn't create window.");

	wptr->activate();

	return *wptr;
}

Window *Window::getByHandle(WindowHandle handle) {

	for (u32 i = 0; i < maxWindows; ++i)
		if (windows[i].activated && windows[i].handle.sameId(handle)) 
			return windows + i;

	return nullptr;
}

void Window::activate() {
	activated = true;
}

void Window::destroy() {
	activated = false;
	handle.destroy();
}

bool Window::isActivated() { return activated; }
u32 Window::getIndex() { return index; }
WindowInfo &Window::getInfo() { return info; }
WindowHandle &Window::getHandle() { return handle; }
InputHandler &Window::getInput() { return input; }
WindowInterface *Window::getInterface() { return wi; }

void Window::notifyUpdate(WindowAction action) {
	handle.update(info, action);
}

void Window::swapBuffers() {
	handle.swapBuffers();
}

void Window::handleBinding(Key key, bool value) {
	input.handleKey(key, value);
}

void Window::handleBinding(Click mouse, bool value) {
	input.handleClick(mouse, value);
}

void Window::update() {

	f64 dt = t.count(lastTick);

	if (wi != nullptr)
		wi->update(dt);

	lastTick = t.getDuration();
	
	if (wi != nullptr)
		wi->render();

	input.update();
}

void Window::init() {
	if (wi != nullptr)
		wi->init();
}

void Window::updateAll() {
	for (u32 i = 0; i < maxWindows; ++i)
		if (windows[i].activated)
			windows[i].update();
}

void Window::initAll() {
	for (u32 i = 0; i < maxWindows; ++i)
		if (windows[i].activated)
			windows[i].init();
}

Window::~Window() {
	if (wi != nullptr) {
		delete wi;
		wi = nullptr;
	}
}

void Window::setInterface(WindowInterface *_wi) {
	if (wi != nullptr)
		delete wi;

	wi = _wi;
	if (wi != nullptr)
		wi->parent = this;
}

#ifdef __WINDOWS__
void Window::waitAll() {
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	initAll();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
			updateAll();
	}
}
#endif