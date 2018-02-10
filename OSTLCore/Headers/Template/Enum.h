#pragma once

#include "Types/Generic.h"
#include "Types/OString.h"
#include "TemplateFuncs.h"
#include "Utils/Log.h"

//Note for enums:
//Please make sure that names are standard C/C++ names (so no spaces/tabs; no starting with numbers, etc.)
//Please make sure that every enum is unique, there is no error yet; though looking up values will not work as expected

namespace oi {

	#define _(...) __VA_ARGS__

	//Structured enum (constexpr)
	#define SEnum(name, structInfo, a0, ...)														\
	struct name##_s {																				\
	structInfo																						\
																									\
	constexpr name##_s operator=(name##_s other) const { return *this; }							\
	};																								\
																									\
	class name {																					\
																									\
	public:																							\
																									\
		static constexpr name##_s a0, __VA_ARGS__;													\
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
		name(OString name) : index(0) {																\
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
		const name##_s &getValue() { return *(ilist + index); }										\
		operator const name##_s&() { return getValue(); }											\
																									\
		u32 getIndex() { return index; }															\
																									\
		const oi::OString getName() { return getNames()[index]; }									\
		operator const oi::OString() { return getName(); }											\
																									\
		static std::vector<name##_s> getValues() {													\
			auto vec = std::vector<name##_s>(length);												\
			vec.assign(ilist, ilist + length - 1);													\
			return vec;																				\
		}																							\
																									\
		static std::vector<oi::OString> getNames() {												\
			static const std::vector<oi::OString> names = initNames();								\
			return names;																			\
		}																							\
																									\
		static const oi::OString &getEnumName() {													\
			static const oi::OString name = #name;													\
			return name;																			\
		}																							\
																									\
		static name get(u32 i) { return name(i); }													\
																									\
		static name find(name##_s k) { return name(k); }											\
																									\
	protected:																						\
																									\
		static const std::vector<oi::OString> initNames() {											\
																									\
			std::vector<oi::OString> res;															\
			std::vector<oi::OString> arr = getArgs().split("=");									\
																									\
			u32 i = 0;																				\
			for (oi::OString temp : arr) {															\
																									\
				if (i == 0){																		\
					OString almostCorrect = temp;													\
					std::vector<oi::OString> csplit2 = almostCorrect.split("(");					\
					res.push_back(csplit2[csplit2.size() - 1]);										\
				}																					\
				else if (i < arr.size() - 1) {														\
					std::vector<oi::OString> csplit = temp.split(",");								\
					oi::OString almostCorrect = csplit[csplit.size() - 1];							\
					std::vector<oi::OString> csplit2 = almostCorrect.split("(");					\
					res.push_back(csplit2[csplit2.size() - 1]);										\
				}																					\
																									\
				++i;																				\
			}																						\
																									\
			return res;																				\
		}																							\
																									\
		static const oi::OString &getArgs() {														\
			static const oi::OString args = (oi::OString(#a0) + ", " + oi::OString(#__VA_ARGS__)).replace(" ", "").replace("\t", "").replace("\r", "").replace("\n", "");	\
			return args;																			\
		}																							\
																									\
	private:																						\
																									\
		u32 index;																					\
																									\
		static constexpr name##_s ilist[] = { a0, __VA_ARGS__ };									\
																									\
		template<u32 i>																				\
		static constexpr u32 getLength(const name##_s(&t)[i]) { return i; }							\
																									\
	public:																							\
																									\
		static constexpr u32 length = getLength(ilist);												\
																									\
	};

	//Data type enum
	#define DEnum(name, type, a0, ...) SEnum(name, type value; constexpr name##_s(type val): value(val) {}; constexpr name##_s(): value((type)0){}; operator type() const { return value; }, a0, __VA_ARGS__)


}