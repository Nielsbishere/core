#ifdef __WINDOWS__

#include "input/mouse.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Mouse::update(Window*, f32) {
	prev = next;
}


#endif