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
WindowExt &Window::getExtension() { return ext; }

void Window::setInterface(WindowInterface *wi) {

	if (this->wi != nullptr)
		delete this->wi;

	this->wi = wi; 

	if (wi != nullptr)
		wi->parent = this;

}

void Window::init() {

	lastTick = Timer::getGlobalTimer().getDuration();
	
	inputHandler.init();
	initPlatform();
	
}

void Window::finalize(){
	if(wi != nullptr){
		if(finalizeCount == 0){
			wi->init();
			wi->initSurface();
		}
		else
			wi->initSurface();
	}
	
	++finalizeCount;
}

void Window::update() {

	if(!initialized || info.size.x == 0 || info.size.y == 0 || info.isMinimized()){
		lastTick = Timer::getGlobalTimer().getDuration();
		hasPrevFrame = false;
		return;
	}

	updatePlatform();

	flp dt = Timer::getGlobalTimer().getDuration() - lastTick;

	if (wi != nullptr)
		wi->update(dt);

	lastTick = Timer::getGlobalTimer().getDuration();

	if (wi != nullptr)
		wi->render();

	hasPrevFrame = true;
	
	inputManager.update();
	inputHandler.update(this, dt);
}

bool Window::hasPreviousFrame() { return hasPrevFrame; }