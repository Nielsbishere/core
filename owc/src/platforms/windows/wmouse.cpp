#include <template/platformdefines.h>

#ifdef __WINDOWS__

#include "input/mouse.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Mouse::update(Window *w, flp dt) {

	WindowInterface *wi = w->getInterface();

	if (wi != nullptr)
		if (prevConnected && !connected)
			wi->onDisconnect(this);
		else if (!prevConnected && connected)
			wi->onConnect(this);

	prev = next;
	prevConnected = connected;

}


#endif