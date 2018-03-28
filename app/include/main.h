#include <window/windowmanager.h>
#include <window/windowinterface.h>

struct Application {
	static void instantiate(void *param = nullptr);
};

class MainInterface : public oi::wc::WindowInterface {

public:

	void init() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;

};