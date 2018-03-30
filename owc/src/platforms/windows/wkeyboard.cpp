#include <template/platformdefines.h>

#ifdef __WINDOWS__

#include "input/keyboard.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Keyboard::update(Window *w, flp dt) {
	prev = next;
}


#endif