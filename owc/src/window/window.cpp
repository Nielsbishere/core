#include "window/window.h"
#include "window/windowinterface.h"
#include "utils/timer.h"
using namespace oi;
using namespace wc;

Window *Window::window = nullptr;

Window::Window(const String &title, const u32 version) : inputHandler(), inputManager(&inputHandler), title(title), version(version) { }

Window::~Window() {
	window = nullptr;
	destroyPointer(wi);
	destroyPointer(viewport);
}

void Window::setInterface(WindowInterface *wif) {

	if (wi == wif)
		return;

	destroyPointer(wi);
	wi = wif;

	if (wi)
		wi->parent = this;

	if (viewport)
		viewport->setInterface(wif);
}

bool Window::update() {

	if (!viewport)
		return false;

	viewport->begin();
	f32 dt = viewport->update();

	if (dt == f32_MAX)
		return false;

	viewport->end();
	
	inputManager.update();
	inputHandler.update(this, dt);
	return true;
}

void Window::wait() {
	while (update());
}