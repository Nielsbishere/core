#include "input/inputmanager.h"
#include "file/filemanager.h"
#include "utils/serialization.h"
using namespace oi;
using namespace wc;

InputAxis::InputAxis(Binding binding, InputAxis1D effect, f32 axisScale, bool delta, f32 threshold) : binding(binding), effect(effect), axisScale(axisScale), delta(delta), threshold(threshold) {}
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

Vec3 InputManager::getAxis(String handle) const {
	auto it = axes.find(handle);
	if (it != axes.end())
		return it->second.value;

	return {};
}

f32 InputManager::getAxis1D(String handle, InputAxis1D ia) const {

	Vec3 val = getAxis(handle);

	switch (ia) {
	case InputAxis1D::X:
		return val.x;

	case InputAxis1D::Y:
		return val.y;

	default:
		return val.z;
	}
}

Vec2 InputManager::getAxis2D(String handle, InputAxes2D ia) const {

	Vec3 val = getAxis(handle);
	u32 iai = (u32) ia;

	u32 axx = iai / 2U;
	u32 axy = (iai % 2U + 1U + axx) % 3U;

	return { val[axx], val[axy] };
}


bool InputManager::load(String path) {

	String file;
	if (!FileManager::get()->read(path, file))
		return Log::error("Couldn't read InputManager file");

	JSON json = file;

	if (json.exists("bindings"))
		for (auto &node : json["bindings"]) {

			std::vector<String> strings;
			node.second.serialize(strings, false);

			for (String str : strings) {

				Binding b(str);

				if (b.getCode() != 0)
					bindState(node.first, b);
				else
					Log::error("Couldn't read binding; invalid identifier");

			}
		}

	if (json.exists("axes"))
		for (auto &node : json["axes"])
			for (auto &nnode : node.second) {

				String bstr = nnode.second.get<String>("binding");
				Binding b(bstr);

				if (b.getCode() == 0) {
					Log::error("Couldn't read axis; invalid identifier");
					continue;
				}

				String effect = nnode.second.get<String>("effect");
				InputAxis1D axis;

				if (effect.equalsIgnoreCase("x")) axis = InputAxis1D::X;
				else if (effect.equalsIgnoreCase("y")) axis = InputAxis1D::Y;
				else if (effect.equalsIgnoreCase("z")) axis = InputAxis1D::Z;
				else {
					Log::error("Couldn't read axis; invalid axis effect");
					continue;
				}

				f32 axisScale = nnode.second.exists("axisScale") ? nnode.second.get<f32>("axisScale") : 1;
				f32 threshold = nnode.second.exists("threshold") ? nnode.second.get<f32>("threshold") : 0;

				bindAxis(node.first, InputAxis(b, axis, axisScale, b.useDelta(), threshold));

			}

	return true;
}

String InputManager::write() const {

	JSON json;

	for (auto &state : states) {

		auto &st = state.second.bindings;
		JSONNode &base = json["bindings"][state.first];

		u32 j = 0;

		for (auto &elem : st) {
			base.set(j, elem.toString());
			++j;
		}

	}

	for (auto &axis : axes) {

		auto &ax = axis.second.bindings;
		JSONNode &base = json["axes"][axis.first];

		u32 j = 0;

		for (auto &elem : ax) {
			base[j].set("binding", elem.binding.toString());
			base[j].set("effect", elem.effect == InputAxis1D::X ? "x" : (elem.effect == InputAxis1D::Y ? "y" : "z"));
			base[j].set("axisScale", elem.axisScale);
			base[j].set("threshold", elem.threshold);
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

			InputState istate = mapped->getState(elem);

			if (istate == InputState::DOWN) {
				st.value = InputState::DOWN;
				break;
			} else if (istate == InputState::RELEASED && st.value == InputState::UP)
				st.value = InputState::RELEASED;
			else if (istate == InputState::PRESSED && st.value == InputState::UP)
				st.value = InputState::PRESSED;
			else if (istate == InputState::PRESSED && st.value == InputState::RELEASED) {
				st.value = InputState::DOWN;
				break;
			} else if (istate == InputState::RELEASED && st.value == InputState::PRESSED) {
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

			f32 val = mapped->getAxis(elem.binding, elem.delta) * elem.axisScale;

			if (std::abs(val) <= elem.threshold)
				continue;

			ax.value[(u32)elem.effect] += val;

		}
	}

}