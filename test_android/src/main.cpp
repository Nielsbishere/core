#include <android_native_app_glue.h>
#include <window/windowmanager.h>
#include <window/windowinterface.h>
using namespace oi::wc;
using namespace oi;

class Itest_android : public WindowInterface {

public:

	void init() override { 
		Log::println("Testing!");
	}
	
	void onInput(InputDevice *device, Binding b, bool down) override {
		Log::println(b.toString());
	}

};

void android_main(struct android_app *state){
	WindowManager manager;
	Window *w = manager.create(WindowInfo("test_android", state));
	w->setInterface(new Itest_android());
	manager.waitAll();
}
