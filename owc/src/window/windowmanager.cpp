#include "window/windowmanager.h"
#include <algorithm>
using namespace oi::wc;

WindowManager *WindowManager::instance = nullptr;

void WindowManager::remove(Window *w) {

	auto it = find(windows.begin(), windows.end(), w);

	if(it != windows.end())
		windows.erase(it);

	delete w;
}

Window *WindowManager::create(WindowInfo info) {
	Window *w = new Window(this, info);
	windows.push_back(w);
	return w;
}

WindowManager::~WindowManager() {

	for (auto ptr : windows)
		delete ptr;

	if (instance == this)
		instance = nullptr;
}

WindowManager::WindowManager() { instance = this; }
WindowManager *WindowManager::get() { return instance; }

void WindowManager::updateAll() {
	for (auto ptr : windows)
		ptr->update();
}

void WindowManager::initAll() {
	for (auto ptr : windows)
		ptr->init();
}

u32 WindowManager::getWindows() { return (u32)windows.size(); }
Window *WindowManager::operator[](u32 i) {

	if (i >= getWindows())
		Log::throwError<WindowManager, 0x0>("Window out of bounds exception");

	return windows[i];
}
