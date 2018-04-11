#include "window/windowinfo.h"
using namespace oi;
using namespace wc;

WindowInfo::WindowInfo(String title, u32 version, void *handle, bool inFocus): version(version), title(title), inFocus(inFocus), handle(handle), pending(WindowAction::NONE) {}

String WindowInfo::getTitle() const { return title; }
Vec2i WindowInfo::getPosition() const { return position; }
Vec2u WindowInfo::getSize() const { return size; }
Window *WindowInfo::getParent() const { return parent; }

void WindowInfo::move(Vec2i newPos) { position = newPos; notify(WindowAction::MOVE); }
void WindowInfo::resize(Vec2u newSize) { size = newSize; notify(WindowAction::RESIZE); }
void WindowInfo::_forceSize(Vec2u newSize) { size = newSize; }

void WindowInfo::focus() { inFocus = true; notify(WindowAction::RESIZE); }

bool WindowInfo::isInFocus() const { return inFocus; }
u32 WindowInfo::getVersion() const { return version; }

void WindowInfo::setParent(Window *w) { parent = w; }
void WindowInfo::notify(WindowAction action) { pending = (WindowAction)((u32)pending | (u32)action); }