#pragma once

#include "types/matrix.h"
#include "utils/log.h"

//Avoid linking for all ostlc's dependencies by forward declaration
#ifndef RAPIDJSON
namespace rapidjson {

	class CrtAllocator;

	template <typename BaseAllocator = CrtAllocator>
	class MemoryPoolAllocator;

	template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
	class GenericValue;

	template<typename CharType = char>
	struct UTF8;

	typedef GenericValue<UTF8<> > Value;

	template <typename Encoding, typename Allocator = MemoryPoolAllocator<>, typename StackAllocator = CrtAllocator>
	class GenericDocument;

	typedef GenericDocument<UTF8<> > Document;

	enum Type {
		kNullType = 0,      //!< null
		kFalseType = 1,     //!< false
		kTrueType = 2,      //!< true
		kObjectType = 3,    //!< object
		kArrayType = 4,     //!< array 
		kStringType = 5,    //!< string
		kNumberType = 6     //!< number
	};

	typedef unsigned SizeType;

}
#define RAPIDJSON
#endif

//Wrappers around rapidjson
namespace oi {

	class JSON;

	template<typename T, bool isNum = std::is_arithmetic<T>::value> struct JSONTypeHelper;
	template<typename T, bool b = std::is_arithmetic<T>::value> struct JSONSerialize;
	template<typename T, typename ...args> struct JSONSerializeAll;
	template<typename T> struct JSONSerializeAll<T>;

	struct JSONNodeUtils {

		static u64 getUInt(rapidjson::Value *node);
		static i64 getInt(rapidjson::Value *node);
		static f64 getFloat(rapidjson::Value *node);
		static bool getBool(rapidjson::Value *node);

		static void setUInt(rapidjson::Value *node, u64 val);
		static void setInt(rapidjson::Value *node, i64 val);
		static void setFloat(rapidjson::Value *node, f64 val);
		static void setBool(rapidjson::Value *node, bool val);

	};

	template<typename T, bool isInt = std::is_integral<T>::value, bool isUInt = std::is_unsigned<T>::value, bool isFloat = std::is_floating_point<T>::value>
	struct JSONNodeObj {

		static T get(rapidjson::Value *node) {
			static_assert(false, "JSONNodeObj<T> not valid");
			return T{};
		}

		static void set(rapidjson::Value *node, T &t) {
			static_assert(false, "JSONNodeObj<T> not valid");
		}

	};

	template<typename T>
	struct JSONNodeObj<T, true, false, false> {
		static T get(rapidjson::Value *node) { return (T)JSONNodeUtils::getInt(node); }
		static void set(rapidjson::Value *node, T &t) { JSONNodeUtils::setInt(node, (i64)t); }
	};

	template<typename T>
	struct JSONNodeObj<T, true, true, false> {
		static T get(rapidjson::Value *node) { return (T)JSONNodeUtils::getUInt(node); }
		static void set(rapidjson::Value *node, T &t) { JSONNodeUtils::setUInt(node, (u64)t); }
	};

	template<typename T>
	struct JSONNodeObj<T, false, false, true> {
		static T get(rapidjson::Value *node) { return (T)JSONNodeUtils::getFloat(node); }
		static void set(rapidjson::Value *node, T &t) { JSONNodeUtils::setFloat(node, (f64)t); }
	};

	template<>
	struct JSONNodeObj<bool, true, true, false> {
		static bool get(rapidjson::Value *node) { return JSONNodeUtils::getBool(node); }
		static void set(rapidjson::Value *node, bool &t) { JSONNodeUtils::setBool(node, t); }
	};

	class JSONNode {

		template<typename T, bool b> friend struct JSONSerialize;
		template<typename T, bool, bool, bool> friend struct JSONNodeObj;

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

		rapidjson::Type getType();

		template<typename T>
		bool isType() {
			return JSONTypeHelper<T>::check(getType());
		}

		bool isObject();
		bool isList();

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
			return JSONNodeObj<T>::get(value);
		}

		template<typename T>
		void _set(T &t) {
			JSONNodeObj<T>::set(value, t);
		}

		void setString(String str);
		String getString();

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