#include "Input/InputManager.h"
#include <Utils/JSON.h>

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

bool InputManager::load(OString path) {

	JSON json = OString::readFromFile(path);

	if (json.exists("bindings")) {

		for (OString handle : json.getMemberIds("bindings")) {

			for (OString id : json.getMemberIds(OString("bindings/") + handle)) {

				Binding b(json.getString(OString("bindings/") + handle + "/" + id));

				if (b.getType() != BindingType::UNDEFINED && b.getCode() != 0) {
					bind(handle, b);
					Log::println(OString("Binding event ") + b.toString());
				}
				else
					Log::error("Couldn't read binding; invalid identifier");

			}

		}

	}

	if (json.exists("axes")) {
		for (OString handle : json.getMemberIds("axes")) {
			for (OString id : json.getMemberIds(OString("axes/") + handle)) {

				OString base = OString("axes/") + handle + "/" + id;

				OString bstr = json.getString(base + "/binding");
				Binding b(bstr);

				if (b.getType() == BindingType::UNDEFINED || b.getCode() == 0) {
					Log::error("Couldn't read axis; invalid identifier");
					continue;
				}

				OString effect = json.getString(base + "/effect");
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
				Log::println(OString("Binding axis ") + b.toString() + " with axis " + effect + " and scale " + axisScale);
			}
		}
	}

	return true;
}

bool InputManager::write(OString path) {

	JSON json;

	for (u32 i = 0; i < (u32)bindings.size(); ++i) {

		OString base = OString("bindings/") + bindings[i].first;
		u32 j = json.getMembers(base);

		json.setString(base + "/" + j, bindings[i].second.toString());
	}

	for (u32 i = 0; i < (u32) axes.size(); ++i) {

		auto &ax = axes[i].second;

		OString base = OString("axes/") + axes[i].first;
		u32 j = json.getMembers(base);

		json.setString(base + "/" + j + "/binding", ax.binding.toString());
		json.setString(base + "/" + j + "/effect", ax.effect == InputAxis1D::X ? "x" : (ax.effect == InputAxis1D::Y ? "y" : "z"));
		json.setFloat(base + "/" + j + "/axisScale", ax.axisScale);
	}

	return json.operator oi::OString().writeToFile(path);
}

InputAxis::InputAxis(Binding _binding, InputAxis1D _effect, f32 _axisScale) : binding(_binding), effect(_effect), axisScale(_axisScale) {}