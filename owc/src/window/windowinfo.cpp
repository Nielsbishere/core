#include "window/windowinfo.h"
using namespace oi;
using namespace wc;

WindowInfo::WindowInfo(String title, void *handle, Vec2u size, Vec2i position, bool inFocus): title(title), size(size), position(position), inFocus(inFocus), handle(handle), pending(WindowAction::NONE) {}

String WindowInfo::getTitle() const { return title; }
Vec2i WindowInfo::getPosition() const { return position; }
Vec2u WindowInfo::getSize() const { return size; }
Window *WindowInfo::getParent() const { return parent; }

void WindowInfo::move(Vec2i newPos) { position = newPos; notify(WindowAction::MOVE); }
void WindowInfo::resize(Vec2u newSize) { size = newSize; notify(WindowAction::RESIZE); }

void WindowInfo::focus() { inFocus = true; notify(WindowAction::RESIZE); }

bool WindowInfo::isInFocus() const { return inFocus; }

void WindowInfo::setParent(Window *w) { parent = w; }
void WindowInfo::notify(WindowAction action) { pending = (WindowAction)((u32)pending | (u32)action); }