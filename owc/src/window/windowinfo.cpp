#include "window/windowinfo.h"
using namespace oi::wc;
using namespace oi;

WindowInfo::WindowInfo(AppExt *handle, bool decorated): handle(handle), decorated(decorated) {}

Vec2i WindowInfo::getPosition() const { return position; }
Window *WindowInfo::getParent() const { return parent; }
AppExt *WindowInfo::getApp() const { return handle; }

bool WindowInfo::isInFocus() const { return inFocus; }
bool WindowInfo::isMinimized() const { return minimized; }
bool WindowInfo::isDecorated() const { return decorated; }

void WindowInfo::setParent(Window *w) { parent = w; }
