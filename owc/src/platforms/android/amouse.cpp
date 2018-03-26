#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "input/mouse.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Mouse::update(Window *w, flp dt) {
	Log::warn("AMouse::update isn't available, as Android doesn't support mice");
}


#endif