#include "Window/WindowInfo.h"
#include "Window/Window.h"
using namespace oi::wc;
using namespace oi;

void WindowInfo::notifyUpdate(WindowAction action) {
	if (parent != nullptr) {
		pending = (WindowAction)((u32)pending | (u32)action);
		parent->notifyUpdate(pending);
		pending = WindowAction::NONE;
	}
}

WindowInfo::WindowInfo(OString _title, Vec2u _size, Vec2i _position, bool _inFocus): title(_title), size(_size), position(_position), inFocus(_inFocus), cursor(0, 0) { }

OString WindowInfo::getTitle() { return title; }
Vec2i WindowInfo::getPosition() { return position; }
Vec2u WindowInfo::getSize() { return size; }
Window *WindowInfo::getParent() { return parent; }
bool WindowInfo::isInFocus() { return inFocus; }

void WindowInfo::move(Vec2i newPos) {
	position = newPos;
	notifyUpdate(WindowAction::MOVE);
}

void WindowInfo::resize(Vec2u newSize) {
	size = newSize;
	notifyUpdate(WindowAction::RESIZE);
}

void WindowInfo::initParent(Window *w) { 
	parent = w; 
	notifyUpdate(WindowAction::NONE);
}

void WindowInfo::focus() {
	inFocus = true;
	notifyUpdate(WindowAction::IN_FOCUS);
}

void WindowInfo::moveCursor(Vec2u pos) {
	cursor = pos;
}

Vec2 WindowInfo::getCursor() {
	return (Vec2(cursor) / Vec2(size - 1)).clamp(0, 1);
}