#include "window/window.h"
#include "window/windowinterface.h"
#include "window/windowmanager.h"
#include "utils/timer.h"
using namespace oi;
using namespace wc;

Window::Window(WindowManager *manager, WindowInfo info) : parent(manager), inputHandler(), inputManager(&inputHandler), info(info) {}
Window::~Window() { if (wi != nullptr) delete wi; destroyPlatform(); }

WindowInfo &Window::getInfo() { return info; }
InputHandler &Window::getInputHandler() { return inputHandler; }
InputManager &Window::getInputManager() { return inputManager; }
WindowInterface *Window::getInterface() { return wi; }
WindowManager *Window::getParent() { return parent; }

void Window::setInterface(WindowInterface *wi) {

	if (this->wi != nullptr)
		delete this->wi;

	this->wi = wi; 

	if (wi != nullptr)
		wi->parent = this;

}

void Window::init() {

	lastTick = Timer::getGlobalTimer().getDuration();

	if(wi != nullptr)
		wi->init();

	inputHandler.init();

	initPlatform();
}

void Window::update() {

	flp dt = Timer::getGlobalTimer().getDuration() - lastTick;

	if (wi != nullptr)
		wi->update(dt);

	lastTick = Timer::getGlobalTimer().getDuration();

	if (wi != nullptr)
		wi->render();

	inputManager.update();
	inputHandler.update(this, dt);

}