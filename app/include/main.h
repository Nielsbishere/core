#include <window/windowmanager.h>
#include <window/windowinterface.h>

struct Application {
	static void instantiate(void *param = nullptr);
};

class MainInterface : public oi::wc::WindowInterface {

public:

	~MainInterface();
	void init() override;
	void initWindow() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String str) override;
	void save(oi::String str) override;
	void update(flp dt) override;

};