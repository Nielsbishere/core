#pragma once
#include <type_traits>

//HAS_FUNC_NAMED detects if a member function signature is present

//Example:
//HAS_FUNC_NAMED(IsSerializable, serialize, void, T*)
//Would generate IsSerializable<T> that checks if the member function "void serialize(T*)" is available
//This means that you can also check for functions that take itself as a parameter

//NOTE: Can give false positives if a type can implicitly cast (like mistake ints for floats)
//		So please keep in mind that this won't give perfect results when casting is at play

//Thanks to /LagMeester4000 for help

#define HAS_FUNC_NAMED(funcName, name, ret, ...)																	\
	template<typename, typename, typename = int, typename ...>														\
	struct T##funcName {																							\
		static constexpr bool value = false;																		\
	};																												\
																													\
	template<typename T, typename Ret, typename ...args>															\
	struct T##funcName<																								\
		T, Ret,																										\
		std::enable_if_t<std::is_same_v<Ret, decltype(std::declval<T>().name(std::declval<args>()...))>, int>,		\
		args...																										\
	> {																												\
		static constexpr bool value = true;																			\
	};																												\
																													\
	template<typename T>																							\
	struct funcName {																								\
		static constexpr bool value = T##funcName<T, ret, int, __VA_ARGS__>::value;									\
	};

//HAS_FUNC_NAMED detects if a member variable is present (even if it's protected or private)

//Example:
//HAS_FIELD_NAMED(HasFieldTest, test, u32)
//Would generate HasFieldTest<T> that checks if the member variable "u32 test" is available
//This means that you can also check for a T* for example (like having a parent)

#define HAS_FIELD_NAMED(funcName, name, ...)															\
	template<typename, typename, typename = int>														\
	struct T##funcName {																				\
		static constexpr bool value = false;															\
	};																									\
																										\
	template<typename T, typename Ret>																	\
	struct T##funcName<T, Ret, 																			\
	std::enable_if_t<std::is_same_v<Ret, decltype(std::declval<T>().name)>, int>						\
	> {																									\
		static constexpr bool value = true;																\
	};																									\
																										\
	template<typename T>																				\
	struct funcName {																					\
		static constexpr bool value = T##funcName<T, __VA_ARGS__, int>::value;							\
	};