#include "main.h"
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(void *param){
	WindowManager manager;
	Window *w = manager.create(WindowInfo("Test window", param));
	w->setInterface(new MainInterface());
	manager.waitAll();
}

//Set up the interface

void MainInterface::initWindow() {
	//Started window again
}

void MainInterface::init(){
	Log::println("Started main interface!");
	//getInputManager().load("resources/settings/input.json");
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::load(String path){ Log::println("Loading"); }
void MainInterface::save(String path){ Log::println("Saving"); }

u64 tick = 0;
void MainInterface::update(flp dt){ if(tick % (5 * 60) == 0) Log::println("Approx 5 secs"); ++tick; }

MainInterface::~MainInterface(){ Log::println("Destroyed"); }