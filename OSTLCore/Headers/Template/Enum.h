#pragma once

#include "Types/Generic.h"
#include "Types/OString.h"
#include "TemplateFuncs.h"
#include "Utils/Log.h"

namespace oi {


	template<typename T, u32 i>
	struct Enumeration {
		const char *name;
		const char *keys[i];
		T values[i];
		u32 len;
	};

	class EnumFactory {

	public:

		template<typename T, typename ...args>
		struct CountNames {
			static constexpr u32 get = CountNames<args...>::get + CountNames<T>::get;
		};

		template<typename T>
		struct CountNames<T> {
			static constexpr u32 get = 0;
		};

		template<>
		struct CountNames<const char*> {
			static constexpr u32 get = 1;
		};

		struct CountNamesArgs {

			template<typename T, typename ...args>
			static constexpr u32 get(T t, args... arg) {
				return CountNames<T, args...>::get;
			}

		};

		template<typename T, bool b = std::is_integral<T>::value>
		struct AddToInt {
			void operator()(T &t) { }
		};

		template<typename T>
		struct AddToInt<T, true> {
			void operator()(T &t) { ++t; }
		};

		template<typename T, bool b = std::is_integral<T>::value>
		struct SetInt {
			void operator()(T &t, T res) { }
		};

		template<typename T>
		struct SetInt<T, true> {
			void operator()(T &t, T res) { t = res; }
		};

		template<u32 k, typename T, u32 l, typename T2>
		static void set(Enumeration<T, l> &w, T &def, u32 &i, u32 &j, T2 t) {
			w.values[j] = (T)t;
			SetInt<T>{}(def, (T)t);
		}

		template<u32 k, typename T, u32 l>
		static void set(Enumeration<T, l> &w, T &def, u32 &i, u32 &j, const char *c) {
			AddToInt<T>{}(def);
			w.keys[i] = c;
			w.values[i] = (T)def;
			j = i;
			++i;
		}

		template<u32 k, typename T, u32 l, typename T2, typename ...args>
		static void set(Enumeration<T, l> &w, T &def, u32 &i, u32 &j, T2 t, args... arg) {
			set<k>(w, def, i, j, t);
			set<k + 1>(w, def, i, j, arg...);
		}

		template<typename T, typename ...args>
		static Enumeration<T, CountNames<args...>::get> make(const char *name, T def, args... arg) {
			Enumeration<T, CountNames<args...>::get> enums;
			u32 i = 0, j = 0;
			set<0>(enums, def, i, j, arg...);
			enums.name = name;
			enums.len = CountNames<args...>::get;
			return enums;
		}

	};

	#define Enum(ename, etype, edefaultVal, earg0, ...)																							\
	class ename {																																\
																																				\
	private:																																	\
																																				\
		static auto &getArr() {																													\
			return oi::EnumFactory::make(#ename, (etype)edefaultVal, earg0, __VA_ARGS__);														\
		}																																		\
																																				\
		static u32 getValidIndex(u32 i) {																										\
			if (size() == 0) oi::Log::throwError<ename, 0x0>("Enum had a size of 0. This is not allowed");										\
			if (i >= getArr().len) return 0;																									\
			return i;																															\
		}																																		\
																																				\
		static const oi::OString __stringify() {																								\
			oi::OString res = "";																												\
			u32 len = size();																													\
			for (u32 i = 0; i < len; ++i)																										\
				res += get(i).toString() + (i == len - 1 ? "" : ", ");																			\
																																				\
			return oi::OString("\"") + getName() + "\": {" + res + "}";																			\
		}																																		\
																																				\
	public:																																		\
																																				\
		static const u32 size() {																												\
			auto &arr = getArr();																												\
			return getArr().len;																												\
		}																																		\
																																				\
		static ename get(u32 i) {																												\
			return ename(getValidIndex(i));																										\
		}																																		\
																																				\
		static oi::OString getKey(u32 i) {																										\
			i = getValidIndex(i);																												\
			return getArr().keys[i];																											\
		}																																		\
																																				\
		static etype getValue(u32 i) {																											\
			i = getValidIndex(i);																												\
			return getArr().values[i];																											\
		}																																		\
																																				\
		static oi::OString getName() {																											\
			return getArr().name;																												\
		}																																		\
																																				\
		static const oi::OString &stringify() {																									\
			static const oi::OString str = __stringify();																						\
			return str;																															\
		}																																		\
																																				\
		static ename findKey(oi::OString key) {																									\
			for (u32 i = 0; i < size(); ++i)																									\
				if (getKey(i) == key)																											\
					return ename(i);																											\
			return ename(0);																													\
		}																																		\
																																				\
		static ename findValue(etype val) {																										\
			for (u32 i = 0; i < size(); ++i)																									\
				if (getValue(i) == val)																											\
					return ename(i);																											\
			return ename(0);																													\
		}																																		\
																																				\
		oi::OString getKey() const { return getKey(in); }																						\
		etype getValue() const { return getValue(in); }																							\
		u32 getIndex() const { return in; }																										\
																																				\
		oi::OString toString() const { return OString("\"") + getKey() + "\": " + getValue(); }													\
																																				\
		ename(): in(0){}																														\
																																				\
		static constexpr u32 length = oi::EnumFactory::CountNamesArgs::get(earg0, __VA_ARGS__);														\
																																				\
	protected:																																	\
																																				\
		ename(u32 index) : in(index) {}																											\
																																				\
		u32 in;																																	\
	};																																			

	#define IEnum(ename, earg0, ...) Enum(ename, i32, -1, earg0, __VA_ARGS__);
	#define UEnum(ename, earg0, ...) Enum(ename, u32, u32_MAX, earg0, __VA_ARGS__);
}