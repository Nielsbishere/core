#include "window/windowinterface.h"
#include "window/window.h"
using namespace oi;
using namespace wc;

Window *WindowInterface::getParent() { return parent; }
InputHandler &WindowInterface::getInputHandler() { return parent->getInputHandler(); }
InputManager &WindowInterface::getInputManager() { return parent->getInputManager(); }