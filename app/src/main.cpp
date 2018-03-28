#include "main.h"
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(void *param){
	Log::println("Application::instantiate called");
	WindowManager manager;
	Window *w = manager.create(WindowInfo("Test window", param));
	w->setInterface(new MainInterface());
	manager.waitAll();
}

//Set up the interface

void MainInterface::init() {
	Log::println("Started main interface!");
	//getInputManager().load("resources/settings/input.json");
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}