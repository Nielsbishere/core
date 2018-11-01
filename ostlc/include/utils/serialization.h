#pragma once

//JSON serialization

#include "utils/json.h"

namespace oi {

	//JSON type helper

	template<typename T, bool isNum>
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

	//For any struct
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

	//For "named arrays" aka, T[] with names
	template<typename T>
	struct JSONSerialize<std::unordered_map<String, T>, false> {

		static void serialize(JSONNode &json, std::unordered_map<String, T> &val, bool save) {
			if (save)
				for (auto &elem : val)
					json[elem.first].serialize(elem.second, true);
			else
				for (auto type : json.children)
					type.second.serialize(val[type.first], false);
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
				json.setString(val);
			else
				val = json.getString();
		}

	};

	template<>
	struct JSONSerialize<const char*, false> {

		static void serialize(JSONNode &json, const char* val, bool save) {
			if (save)
				json.setString(val);
			else
				Log::throwError<JSONSerialize<const char*, false>, 0x0>("JSONSerialize read not possible with a char*");
		}

	};

	template<typename T, typename ...args>
	struct JSONSerializeAll {

		static void serialize(JSONNode *node, const String *member, bool save, T &t, args&... arg) {
			node->serialize(*member, t, save);
			JSONSerializeAll<args...>::serialize(node, member + 1, save, arg...);
		}

	};

	template<typename T>
	struct JSONSerializeAll<T> {

		static void serialize(JSONNode *node, const String *member, bool save, T &t) {
			node->serialize(*member, t, save);
		}

	};

}

/*
Usage:
struct X {

u32 a, b, c;
f32 d, e, f;

ose(X, 0, a, b, c, d, e, f);

};
*/
#define ose(className, version, ...)												\
																					\
static std::vector<oi::String> _initMembers() {										\
																					\
	std::vector<oi::String> members = oi::String(#__VA_ARGS__).split(",");			\
	for (u32 i = 0; i < (u32)members.size(); ++i)									\
		members[i] = members[i].trim();												\
																					\
	members.insert(members.begin(), "structVersion");								\
	return members;																	\
}																					\
																					\
static const std::vector<oi::String> &getMembers() {								\
	static const std::vector<oi::String> members = _initMembers();					\
	return members;																	\
}																					\
																					\
static constexpr u32 getCurrentVersion() { return (u32)version; }					\
																					\
u32 structVersion = getCurrentVersion();											\
																					\
static const oi::String &getClass() {												\
	static const oi::String clsName = #className;									\
	return clsName;																	\
}																					\
																					\
template<typename T>																\
void serialize(T &t, bool save){													\
	t.serializeAll(getMembers(), save, structVersion, __VA_ARGS__);					\
}																					\
																					\
void toJSON(oi::JSON &json) { serialize(json, true); }								\
void fromJSON(oi::JSON &json) { serialize(json, false); }							\
void toString(oi::String &str) {													\
	oi::JSON json;																	\
	toJSON(json);																	\
	str = json.toString(false);														\
}																					\
void fromString(oi::String &str) {													\
	oi::JSON json = str;															\
	fromJSON(json);																	\
}