#include "main.h"
#include <window/windowmanager.h>

#include "graphics/commandlist.h"

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(void *param){
	WindowManager manager;
	Window *w = manager.create(WindowInfo(__PROJECT_NAME__, 1, param));
	w->setInterface(new MainInterface());
	manager.waitAll();
}

//Set up the interface

void MainInterface::initScene(){

	Log::println("Started main interface!");
	//getInputManager().load("resources/settings/input.json");

	cmdList = g.create(CommandListInfo());

}

void MainInterface::renderScene(){
	cmdList->begin();
	cmdList->begin(g.getBackBuffer(), Vec4d(0.25, 0.5, 1, 1) * (sin(getDuration()) * 0.5 + 0.5));
	cmdList->end(g.getBackBuffer());
	cmdList->end();
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::load(String path){ Log::println("Loading"); }
void MainInterface::save(String path){ Log::println("Saving"); }

void MainInterface::update(flp dt){  }

MainInterface::~MainInterface(){ Log::println("Destroyed"); }