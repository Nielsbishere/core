#ifdef __WINDOWS__

#include "input/keyboard.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Keyboard::update(Window*, f32) {
	prev = next;
}


#endif