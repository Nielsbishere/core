#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "input/keyboard.h"
#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi::wc;

void Keyboard::update(Window *w, flp dt) {

	WindowInterface *wi = w->getInterface();

	if (wi != nullptr){
		if (prevConnected && !connected){
			wi->onDisconnect(this);
		} else if (!prevConnected && connected){
			wi->onConnect(this);
		}
	}
	
	prev = next;
	prevConnected = connected;

}


#endif