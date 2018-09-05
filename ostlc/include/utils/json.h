#pragma once

#include "api/rapidjson/document.h"
#include "types/string.h"
#include "types/matrix.h"
#include "utils/log.h"

namespace oi {

	class JSON;

	template<typename T, bool isNum = std::is_arithmetic<T>::value>
	struct JSONTypeHelper {

		static bool check(rapidjson::Type type) {
			return type == rapidjson::Type::kStringType;
		}

	};

	template<typename T>
	struct JSONTypeHelper<T, true> {

		static bool check(rapidjson::Type type) {
			return type == rapidjson::Type::kNumberType;
		}

	};

	template<>
	struct JSONTypeHelper<bool, true> {

		static bool check(rapidjson::Type type) {
			return type == rapidjson::Type::kTrueType || type == rapidjson::Type::kFalseType;
		}

	};

	template<typename T, bool b = std::is_arithmetic<T>::value> struct JSONSerialize;

	class JSONNode {

		template<typename T, bool b> friend struct JSONSerialize;

	public:

		JSONNode(rapidjson::Document *json, rapidjson::Value *value) : json(json), value(value) {}
		JSONNode() : JSONNode(nullptr, nullptr) {}

		template<typename T>
		void serialize(String var, T &val, bool save) {
			JSONSerialize<T>::serialize(operator[](var), val, save);
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

	//For any function
	template<typename T, bool isDataType>
	struct JSONSerialize {

		static void serialize(JSONNode &json, T &val, bool save) {
			val.serialize(json, save);
		}

	};

	//For vectors
	template<typename T>
	struct JSONSerialize<std::vector<T>, false> {

		static void serialize(JSONNode &json, std::vector<T> &val, bool save) {

			if (!save)
				val.resize(json.getMembers());

			for (u32 i = 0; i < (u32)val.size(); ++i)
				JSONSerialize<T>::serialize(json[i], val[i], save);

		}

	};

	template<typename T, u32 n>
	struct JSONSerialize<oi::TVec<T, n>, false> {

		static void serialize(JSONNode &json, oi::TVec<T, n> &val, bool save) {
			for (u32 i = 0; i < n; ++i)
				JSONSerialize<T>::serialize(json[i], val[i], save);
		}

	};

	template<typename T, u32 x, u32 y>
	struct JSONSerialize<oi::TMatrix<T, x, y>, false> {
		static void serialize(JSONNode &json, oi::TMatrix<T, x, y> &val, bool save) {
			for (u32 i = 0; i < x * y; ++i)
				JSONSerialize<T>::serialize(json[i % x][i / x], val[i], save);
		}
	};

	//For data types
	template<typename T>
	struct JSONSerialize<T, true> {

		static void serialize(JSONNode &json, T &val, bool save) {
			if (save)
				json._set(val);
			else
				val = json._get<T>();
		}

	};

	//For strings
	template<>
	struct JSONSerialize<String, false> {

		static void serialize(JSONNode &json, String &val, bool save) {
			if (save)
				json.value->SetString(val.toCString(), val.size());
			else
				val = json.value->GetString();
		}

	};

	template<>
	struct JSONSerialize<const char*, false> {

		static void serialize(JSONNode &json, const char* val, bool save) {
			if (save)
				json.value->SetString(val, (rapidjson::SizeType) strlen(val));
			else
				Log::throwError<JSONSerialize<const char*, false>, 0x0>("JSONSerialize read not possible with a char*");
		}

	};

}