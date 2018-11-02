#pragma once

#include <string.h>
#include "types/generic.h"
#include "types/string.h"
#include "utils/log.h"
#include "templatefuncs.h"

//Note for enums:
//Please make sure that names are standard C/C++ names (so no spaces/tabs; no starting with numbers, etc.)
//Please make sure that every enum value is unique, there is no error yet; though looking up values will not work as expected

//These types of enums are called 'structured enums' or 'enum struct'
//Because you can use any struct that is constexpr to define it

namespace oi {

	#define _(...) __VA_ARGS__

	//Structured enum (constexpr)
	#define SEnum(name, structInfo, a0, ...)														\
	struct name##_s {																				\
	structInfo																						\
																									\
	constexpr name##_s operator=(name##_s) const { return *this; }									\
	};																								\
																									\
	class name {																					\
																									\
	public:																							\
																									\
		static constexpr name##_s a0, __VA_ARGS__;													\
																									\
	private:																						\
																									\
		static constexpr name##_s ilist[] = { a0, __VA_ARGS__ };									\
																									\
	public:																							\
																									\
		static constexpr u32 length = (u32)(sizeof(ilist) / sizeof(name##_s));						\
																									\
		name(u32 _index = 0) : index(_index) {														\
			if (index >= length) index = 0;															\
		}																							\
																									\
		name(const name##_s val) : index(0) {														\
			for (u32 i = 0; i < length; ++i)														\
				if (memcmp(ilist + i, &val, sizeof(val)) == 0) {									\
					index = i;																		\
					break;																			\
				}																					\
		}																							\
																									\
		name(oi::String name) : index(0) {															\
																									\
			for (u32 i = 0; i < length; ++i)														\
			if (getNames()[i] == name) {															\
				index = i;																			\
				break;																				\
			}																						\
																									\
		}																							\
																									\
		bool operator==(const name &other) const {													\
			return index == other.index;															\
		}																							\
																									\
		bool operator==(const name##_s &other) const {												\
			return index == name(other).index;														\
		}																							\
																									\
		bool operator!=(const name &other) const {													\
			return index != other.index;															\
		}																							\
																									\
		bool operator!=(const name##_s &other) const {												\
			return index != name(other).index;														\
		}																							\
																									\
		const name##_s &getValue() const { return *(ilist + index); }								\
		operator const name##_s&() const { return getValue(); }										\
																									\
		u32 getIndex() const { return index; }														\
																									\
		const oi::String getName() const { return getNames()[index]; }								\
																									\
		static std::vector<name##_s> getValues() {													\
			auto vec = std::vector<name##_s>(length);												\
			vec.assign(ilist, ilist + length - 1);													\
			return vec;																				\
		}																							\
																									\
		static const std::vector<oi::String> &getNames() {											\
			static const std::vector<oi::String> names = initNames();								\
			return names;																			\
		}																							\
																									\
		static const oi::String &getEnumName() {													\
			static const oi::String name = #name;													\
			return name;																			\
		}																							\
																									\
		static name get(u32 i) { return name(i); }													\
																									\
		static name find(name##_s k) { return name(k); }											\
																									\
	protected:																						\
																									\
		static const std::vector<oi::String> initNames() {											\
																									\
			std::vector<oi::String> res;															\
			std::vector<oi::String> arr = getArgs().split("=");										\
																									\
			u32 i = 0;																				\
			for (oi::String &temp : arr) {															\
																									\
				if (i == 0){																		\
					std::vector<oi::String> csplit2 = temp.split("(");								\
					res.push_back(csplit2[csplit2.size() - 1]);										\
				}																					\
				else if (i < arr.size() - 1) {														\
					std::vector<oi::String> csplit = temp.split(",");								\
					oi::String almostCorrect = csplit[csplit.size() - 1];							\
					std::vector<oi::String> csplit2 = almostCorrect.split("(");						\
					res.push_back(csplit2[csplit2.size() - 1]);										\
				}																					\
																									\
				++i;																				\
			}																						\
																									\
			return res;																				\
		}																							\
																									\
		static const oi::String &getArgs() {														\
			static const oi::String args = (oi::String(#a0) + ", " + oi::String(#__VA_ARGS__)).replace(" ", "").replace("\t", "").replace("\r", "").replace("\n", "");	\
			return args;																			\
		}																							\
																									\
	private:																						\
																									\
		u32 index;																					\
																									\
	};

	//Data type enum
	#define DEnum(name, type, a0, ...) SEnum(name, type value; constexpr name##_s(type val): value(val) {}; constexpr name##_s(): value((type)0){}; operator type() const { return value; }, a0, __VA_ARGS__)

	#define UEnum(name, a0, ...) DEnum(name, u32, a0, __VA_ARGS__)
	#define IEnum(name, a0, ...) DEnum(name, i32, a0, __VA_ARGS__)

}