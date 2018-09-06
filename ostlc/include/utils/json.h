#pragma once

#include "api/rapidjson/document.h"
#include "types/matrix.h"
#include "utils/log.h"

namespace oi {

	class JSON;

	template<typename T, bool isNum = std::is_arithmetic<T>::value> struct JSONTypeHelper;
	template<typename T, bool b = std::is_arithmetic<T>::value> struct JSONSerialize;
	template<typename T, typename ...args> struct JSONSerializeAll;
	template<typename T> struct JSONSerializeAll<T>;

	class JSONNode {

		template<typename T, bool b> friend struct JSONSerialize;

	public:

		JSONNode(rapidjson::Document *json, rapidjson::Value *value) : json(json), value(value) {}
		JSONNode() : JSONNode(nullptr, nullptr) {}

		template<typename T>
		void serialize(String var, T &val, bool save) {
			JSONSerialize<T>::serialize(operator[](var), val, save);
		}

		template<typename ...args>
		void serializeAll(const std::vector<String> &members, bool save, args&... arg) {
			JSONSerializeAll<args...>::serialize(this, members.data(), save, arg...);
		}

		template<typename T>
		void set(String var, const T value) {
			JSONSerialize<T>::serialize(operator[](var), (T&)value, true);
		}

		template<typename T>
		T get(String var) {
			T val;
			JSONSerialize<T>::serialize(operator[](var), val, false);
			return val;
		}

		template<typename T>
		void serialize(T &val, bool save) {
			JSONSerialize<T>::serialize(*this, val, save);
		}

		template<typename T>
		void set(const T value) {
			JSONSerialize<T>::serialize(*this, (T&)value, true);
		}

		template<typename T>
		T get() {
			T val;
			JSONSerialize<T>::serialize(*this, val, false);
			return val;
		}

		JSONNode &operator[](String var);
		const JSONNode &operator[](String var) const;

		u32 getMembers() const;

		bool exists(String var) const;
		bool mkdir(String var);

		auto begin() { return children.begin(); }
		auto end() { return children.end(); }

		String toString(bool pretty = true) const;

		bool isEmpty();
		bool canChangeType();

		template<typename T>
		bool isType() {
			return JSONTypeHelper<T>::check(value == nullptr ? json->GetType() : value->GetType());
		}

		bool isObject() {
			return value == nullptr || value->IsObject();
		}

		bool isList() {
			return value == nullptr || value->IsArray();
		}

	protected:

		void reconstruct();

		template<typename T>
		void _serialize(T &t, bool save) {
			if (save) {
				if (isType<T>() || canChangeType())
					_set(t);
			}
			else
				if (isType<T>())
					t = _get<T>();
		}

		template<typename T>
		T _get() {
			return value->Get<T>();
		}

		template<typename T>
		void _set(T &t) {
			value->Set(t);
		}

		rapidjson::Document *json;
		rapidjson::Value *value;

		std::unordered_map<String, JSONNode> children;

	};

	class JSON : public JSONNode {

	public:

		JSON(const String fromString);
		JSON();
		~JSON();

		JSON(const JSON &other) { copy(other); }
		JSON &operator=(const JSON &other) { copy(other); return *this; }

	private:

		void copy(const JSON &other);

	};

}