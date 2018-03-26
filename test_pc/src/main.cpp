#include "window/windowmanager.h"
#include "window/windowinterface.h"
using namespace oi::wc;
using namespace oi;

class Itest_pc : public WindowInterface {

public:

	void init() override {
		Log::println("Testing!");
		//getInputManager().load("resources/settings/input.json");
	}
	
	void onInput(InputDevice *device, Binding b, bool down) override {
		Log::println(b.toString());
	}

};

int main() {

	WindowManager manager;
	Window *w = manager.create(WindowInfo("test_pc"));
	w->setInterface(new Itest_pc());
	manager.waitAll();

	return 0;
}
