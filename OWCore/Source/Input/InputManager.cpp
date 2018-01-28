#include "Input/InputManager.h"

using namespace oi::wc;
using namespace oi;

InputManager::InputManager(InputHandler &_handler): handler(_handler) { }

bool InputManager::isDown(OString handle) { 

	auto bound = getBound(handle); 

	for (auto b : bound) {
		if (handler.isDown(b))
			return true;
	}

	return false;
}

bool InputManager::isUp(OString handle) {

	auto bound = getBound(handle);

	for (auto b : bound) {
		if (!handler.isUp(b))
			return false;
	}

	return true;
}

bool InputManager::isPressed(OString handle) { 

	auto bound = getBound(handle);

	bool result = false;

	for (auto b : bound) {

		if (handler.isDown(b) || handler.isReleased(b))
			return false;

		if(handler.isPressed(b))
			result = true;
	}

	return result;

}

bool InputManager::isReleased(OString handle) { 

	auto bound = getBound(handle);

	bool result = false;

	for (auto b : bound) {

		if (!(handler.isUp(b) || handler.isReleased(b)))
			return false;

		if (handler.isReleased(b))
			result = true;
	}

	return result;
}

void InputManager::bind(OString handle, Binding binding) {
	bindings.push_back({ handle, binding });
}

void InputManager::unbind(OString handle, Binding binding) {

	for (u32 i = (u32)bindings.size() - 1; i != u32_MAX; --i)
		if (bindings[i].first == handle && bindings[i].second == binding)
			bindings.erase(bindings.begin() + i);

}

void InputManager::unbind(OString handle) {

	for (u32 i = (u32)bindings.size() - 1; i != u32_MAX; --i)
		if (bindings[i].first == handle) 
			bindings.erase(bindings.begin() + i);

}

std::vector<Binding> InputManager::getBound(OString handle) { 

	std::vector<Binding> bound;

	for (auto b : bindings)
		if (b.first == handle)
			bound.push_back(b.second);

	return bound;
}

void InputManager::bindAxis(OString handle, InputAxis axis) {
	axes.push_back({ handle, axis });
}

Vec3 InputManager::getAxis(OString handle) {

	Vec3 result;

	for(auto b : axes)
		if (b.first == handle) {

			switch (b.second.effect) {

			case InputAxis1D::X:
				result += { b.second.axisScale * handler.getAxis(b.second.binding), 0, 0 };
				break;

			case InputAxis1D::Y:
				result += { 0, b.second.axisScale * handler.getAxis(b.second.binding), 0 };
				break;

			default:
				result += { 0, 0, b.second.axisScale * handler.getAxis(b.second.binding) };
				break;
			}

		}

	return result;
}

Vec2 InputManager::getAxis2D(OString handle, InputAxes2D ia) {

	Vec3 axis = getAxis(handle);

	switch (ia) {

	case InputAxes2D::XY:
		return { axis.x(), axis.y() };

	case InputAxes2D::YZ:
		return { axis.y(), axis.z() };

	default:
		return { axis.x(), axis.z() };

	}
}

f32 InputManager::getAxis1D(OString handle, InputAxis1D ia) {

	Vec3 axis = getAxis(handle);

	switch (ia) {

	case InputAxis1D::X:
		return axis.x();

	case InputAxis1D::Y:
		return axis.y();

	default:
		return axis.z();

	}
}

bool InputManager::load(OString json) {
	//TODO:
	return false;
}

OString InputManager::write() {

	OString start = "{ \"bindings\": { ";

	auto cbindings = bindings;
	auto caxes = axes;

	std::sort(cbindings.begin(), cbindings.end());
	std::sort(caxes.begin(), caxes.end());

	OString prevBinding = "";
	u32 bindingCount = 0;

	u32 elemCount = 0;

	for (u32 i = 0; i < (u32)cbindings.size(); ++i) {

		if (prevBinding != cbindings[i].first) {
			start += (bindingCount == 0 ? OString("") : " ], ") + "\"" + cbindings[i].first + "\": [ ";
			++bindingCount;
			elemCount = 0;
			prevBinding = cbindings[i].first;
		}

		start += (elemCount == 0 ? OString("") : ", ") + OString("\"") + cbindings[i].second.toString() + "\"";
		++elemCount;

	}

	start += (elemCount != 0 ? OString(" ] ") : "") + " }, \"axes\": { ";

	prevBinding = "";
	bindingCount = elemCount = 0;

	for (u32 i = 0; i < (u32)caxes.size(); ++i) {

		if (prevBinding != caxes[i].first) {
			start += (bindingCount == 0 ? OString("") : " ], ") + "\"" + caxes[i].first + "\": [ ";
			++bindingCount;
			elemCount = 0;
			prevBinding = caxes[i].first;
		}

		start += (elemCount == 0 ? OString("") : ", ") + caxes[i].second.toString();
		++elemCount;

	}

	return start + (elemCount != 0 ? OString(" ] ") : "") + " } }";
}

InputAxis::InputAxis(Binding _binding, InputAxis1D _effect, f32 _axisScale) : binding(_binding), effect(_effect), axisScale(_axisScale) {}

OString InputAxis::toString() {
	return OString("{ \"binding\": \"") + binding.toString() + "\", \"effect\": " + (effect == InputAxis1D::X ? "\"x\"" : (effect == InputAxis1D::Y ? "\"y\"" : "\"z\"")) + ", \"axisScale\": " + axisScale + " }";
}