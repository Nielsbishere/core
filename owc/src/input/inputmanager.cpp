#include "input/inputmanager.h"
#include "file/filemanager.h"
#include <utils/json.h>
using namespace oi;
using namespace wc;

InputAxis::InputAxis(Binding binding, InputAxis1D effect, flp axisScale) : binding(binding), effect(effect), axisScale(axisScale) {}
InputManager::InputManager(InputHandler *handler) : handler(handler) {}

bool InputManager::contains(String handle) const {
	return containsState(handle) || containsAxis(handle);
}

void InputManager::clear(String handle) {
	clearState(handle);
	clearAxis(handle);
}

void InputManager::clear() {
	states.clear();
	axes.clear();
}

bool InputManager::containsState(String handle) const {
	return states.find(handle) != states.end();
}

bool InputManager::containsAxis(String handle) const {
	return axes.find(handle) != axes.end();
}

void InputManager::bindState(String handle, Binding binding) {
	states[handle].bindings.push_back(binding);
}

void InputManager::clearState(String handle) {
	auto it = states.find(handle);
	if (it != states.end())
		states.erase(it);
}

void InputManager::bindAxis(String handle, InputAxis axis) {
	axes[handle].bindings.push_back(axis);
}

void InputManager::clearAxis(String handle) {
	auto it = axes.find(handle);
	if (it != axes.end())
		axes.erase(it);
}

InputState InputManager::getState(String handle) const {
	auto it = states.find(handle);
	if (it != states.end())
		return it->second.value;
	return InputState::UP;
}

bool InputManager::isDown(String handle) const { return getState(handle) == InputState::DOWN; }
bool InputManager::isPressed(String handle) const { return getState(handle) == InputState::PRESSED; }
bool InputManager::isReleased(String handle) const { return getState(handle) == InputState::RELEASED; }
bool InputManager::isUp(String handle) const { return getState(handle) == InputState::UP; }

Vec3 InputManager::getAxis(String handle, bool clamp) const {
	auto it = axes.find(handle);
	if (it != axes.end())
		return clamp ? it->second.value.clamp((flp)-1, (flp)1) : it->second.value;
	return {};
}

flp InputManager::getAxis1D(String handle, InputAxis1D ia, bool clamp) const {

	Vec3 val = getAxis(handle, clamp);

	switch (ia) {
	case InputAxis1D::X:
		return val.x;

	case InputAxis1D::Y:
		return val.y;

	default:
		return val.z;
	}
}

Vec2 InputManager::getAxis2D(String handle, InputAxes2D ia, bool clamp) const {

	Vec3 val = getAxis(handle, clamp);
	u32 iai = (u32) ia;

	u32 axx = iai / 2U;
	u32 axy = (iai % 2U + 1U + axx) % 3U;

	return { val[axx], val[axy] };
}


bool InputManager::load(String path) {

	String str;
	if (!FileManager::get()->read(path, str))
		return Log::error("Couldn't read InputManager file");

	JSON json = str;

	if (json.exists("bindings"))
		for (String handle : json.getMemberIds("bindings"))
			for (String id : json.getMemberIds(String("bindings/") + handle)) {

				String bstr = json.getString(String("bindings/") + handle + "/" + id);
				Binding b(bstr);

				if (b.getCode() != 0)
					bindState(handle, b);
				else
					Log::error("Couldn't read binding; invalid identifier");

			}

	if (json.exists("axes"))
		for (String handle : json.getMemberIds("axes"))
			for (String id : json.getMemberIds(String("axes/") + handle)) {

				String base = String("axes/") + handle + "/" + id;

				String bstr = json.getString(base + "/binding");
				Binding b(bstr);

				if (b.getCode() == 0) {
					Log::error("Couldn't read axis; invalid identifier");
					continue;
				}

				String effect = json.getString(base + "/effect");
				InputAxis1D axis;

				if (effect.equalsIgnoreCase("x")) axis = InputAxis1D::X;
				else if (effect.equalsIgnoreCase("y")) axis = InputAxis1D::Y;
				else if (effect.equalsIgnoreCase("z")) axis = InputAxis1D::Z;
				else {
					Log::error("Couldn't read axis; invalid axis effect");
					continue;
				}

				f32 axisScale = json.getFloat(base + "/axisScale", 1.f);

				bindAxis(handle, InputAxis(b, axis, axisScale));

			}

	return true;
}

String InputManager::write() const {

	JSON json;

	for (auto &state : states) {

		auto &st = state.second.bindings;
		String base = String("bindings/") + state.first;

		u32 j = 0;

		for (auto &elem : st) {
			json.setString(base + "/" + j, elem.toString());
			++j;
		}

	}

	for (auto &axis : axes) {

		auto &ax = axis.second.bindings;
		String base = String("axes/") + axis.first;

		u32 j = 0;

		for (auto &elem : ax) {
			json.setString(base + "/" + j + "/binding", elem.binding.toString());
			json.setString(base + "/" + j + "/effect", elem.effect == InputAxis1D::X ? "x" : (elem.effect == InputAxis1D::Y ? "y" : "z"));
			json.setFloat(base + "/" + j + "/axisScale", elem.axisScale);
			++j;
		}

	}

	return json.toString();
}

bool InputManager::write(String path) const {
	return FileManager::get()->write(path, write());
}

void InputManager::update() {

	for (auto &state : states) {

		auto &st = state.second;
		st.value = InputState::UP;

		for (auto &elem : st.bindings) {

			InputDevice *mapped = handler->getDevice(elem);

			if (mapped == nullptr) continue;

			InputState state = mapped->getState(elem);

			if (state == InputState::DOWN) {
				st.value = InputState::DOWN;
				break;
			}

			else if (state == InputState::RELEASED && st.value == InputState::UP)
				st.value = InputState::RELEASED;

			else if (state == InputState::PRESSED && st.value == InputState::UP)
				st.value = InputState::PRESSED;

			else if (state == InputState::PRESSED && st.value == InputState::RELEASED) {
				st.value = InputState::DOWN;
				break;
			}

			else if (state == InputState::RELEASED && st.value == InputState::PRESSED) {
				st.value = InputState::DOWN;
				break;
			}

		}

	}

	for (auto &axis : axes) {

		auto &ax = axis.second;
		ax.value = {};

		for (auto &elem : ax.bindings) {

			InputDevice *mapped = handler->getDevice(elem.binding);

			if (mapped == nullptr) continue;

			flp val = mapped->getAxis(elem.binding) * elem.axisScale;

			ax.value[(u32)elem.effect] += val;

		}
	}

}