#include "window/windowinfo.h"
using namespace oi::wc;
using namespace oi;

WindowInfo::WindowInfo(String title, u32 version, AppExt *handle, bool inFocus): version(version), title(title), inFocus(inFocus), handle(handle), pending(WindowAction::NONE) {}

String WindowInfo::getTitle() const { return title; }
Vec2i WindowInfo::getPosition() const { return position; }
Vec2u WindowInfo::getSize() const { return size; }
Window *WindowInfo::getParent() const { return parent; }
AppExt *WindowInfo::getApp() const { return handle; }
bool WindowInfo::isFullScreen() const { return fullScreen; }

void WindowInfo::setFullScreen(bool isFullScreen) { 
	fullScreen = isFullScreen;
	notify(WindowAction::FULL_SCREEN); 
}

void WindowInfo::toggleFullScreen() {
	setFullScreen(!fullScreen);
}

void WindowInfo::focus() { inFocus = true; notify(WindowAction::IN_FOCUS); }

bool WindowInfo::isInFocus() const { return inFocus; }
bool WindowInfo::isMinimized() const { return minimized; }
u32 WindowInfo::getVersion() const { return version; }

void WindowInfo::setParent(Window *w) { parent = w; }
void WindowInfo::notify(WindowAction action) { pending = (WindowAction)((u32)pending | (u32)action); }