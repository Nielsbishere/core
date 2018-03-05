#include "OECore/OEDependencies.h"
#include <Window/Window.h>
#include <Graphics/GraphicsManager.h>
#include "Engine/EngineInterface.h"

using namespace oi::wc;
using namespace oi::gc;
using namespace oi::ec;
using namespace oi;

int main() {

	Window &w = Window::create(WindowInfo("Engine test"));

	GraphicsManager gm;

	w.setInterface(new EngineInterface(gm));
	w.waitAll();

	return 0;
}