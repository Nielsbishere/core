#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi;
using namespace wc;

Window *WindowInterface::getParent() { return parent; }
InputHandler &WindowInterface::getInputHandler() { return parent->getInputHandler(); }
InputManager &WindowInterface::getInputManager() { return parent->getInputManager(); }

Keyboard *WindowInterface::getKeyboard() { return getInputHandler().getKeyboard(); }
Mouse *WindowInterface::getMouse() { return getInputHandler().getMouse(); }
Controller *WindowInterface::getController(u32 id) { return getInputHandler().getController(id); }