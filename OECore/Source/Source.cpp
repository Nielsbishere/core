#include "OECore/OEDependencies.h"
#include <Window/Window.h>
#include <Graphics/Graphics.h>
#include "Engine/EngineInterface.h"

using namespace oi::wc;
using namespace oi::gc;
using namespace oi::ec;
using namespace oi;

int main() {

	Window &w = Window::create(WindowInfo("Engine test"));

	Graphics::startGraphics();

	w.setInterface(new EngineInterface());
	w.waitAll();

	Graphics::endGraphics();

	return 0;
}