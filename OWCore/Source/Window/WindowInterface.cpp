#include "Window/WindowInterface.h"
#include "Window/Window.h"
using namespace oi::wc;
using namespace oi;

Window *WindowInterface::getParent() { return parent; }
InputHandler &WindowInterface::getInputHandler() { return parent->getInput(); }
InputManager &WindowInterface::getInput() { return parent->getInputManager(); }