#include "window/windowmanager.h"
#include "window/windowinterface.h"
using namespace oi::wc;
using namespace oi;

class TestInterface : public WindowInterface {

public:

	void init() override {
		getInputManager().load("resources/settings/input.json");
	}
	
	void onInput(InputDevice *device, Binding b, bool down) override {
		Log::println(b.toString());
	}

};

int main() {

	WindowManager manager;
	Window *w = manager.create(WindowInfo("Test window"));
	w->setInterface(new TestInterface());
	manager.waitAll();

	return 0;
}