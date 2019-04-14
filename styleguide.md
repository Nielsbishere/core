# Oxsomi styleguide

This style guide applies to everything written in this library, other libraries and forks can use whatever they want.

## Headers and source files

- Header files should have as little code as possible, only one-liners like getters and setters can be provided in the header file (with exception to template functions). They should provide a quick and easy overview of what the class represents.
- Template functions should be defined at the bottom instead of the top of the header, to make sure the class's overview isn't cluttered. (with exception of one-liners).

## File names

- Use .hpp and .cpp for C++ header and source files, use .h and .c for C header and source files.

- Separate source and header files into the project's root directory (src/ and include/).

## Includes

- Includes that are from your library (or directly related to your library) have to use `""`, while other includes use `<>`.
- Try to use as less includes as possible, always try to predeclare if it's possible, to avoid longer build times.
- Always use pragma once in headers, not include guards.

## Namespaces

- Always wrap your content into namespaces, unless you have a valid reason to "pollute" the global namespace (like adding typedefs that should be available to all).
- Don't pollute namespaces; if you're creating functions, try putting them into a struct and making them static instead; or just use a custom namespace.

## Types, defines & enums

- Use 'using' not 'typedef'
- Types should be named with PascalCase, and if they are template functions used as helpers, they should be prefixed with T. TCopyArray is a template function that helps copying data from an array, while "Array" is a templated class meant for the end-user.
- NEVER use 'int', 'long' or short', use the types 'i32', 'i64', 'i16' or 'i8'. For unsigned use 'u32', 'u64', 'u16' or 'u8'. For floats use 'f64', 'f32'.
- Whenever talking about a size; use `size_t`.
  - Avoid size_t when using serialized structs, as 64 and 32 bit uints could be used, providing an invalid memory layout.
- Avoid using std types, as our types can do additional optimization.
- For macros or regular defines, avoid using common names; such as near, far, min, max, etc. 
- Try to avoid using the preprocessor as much as possible (this includes ifdefs), only use ifdefs if there's no way to do it cleanly through CMake and abstraction.
  - Avoid using defines, only use them if really necessary. Use constexpr instead, but try avoid hardcoding if possible.
- Avoid using `_` as a prefix, as this is commonly used by the preprocessor
  - `__X__` is reserved for CMake variables; e.g. `__WINDOWS__`, `__ANDROID__`, `__LINUX__`, etc.
- Enums should always use enum class : T, where T is the low level type you want it to represent. Though enum class is not required when it doesn't pollute the namespace (using an enum in a class instead of enum class in a class is fine).
- Enums should have CAPITALIZED_NAMES. The exception is Oxsomi Enums, which follow Oxsomi_enum_casing, since these names and values are often exposed to the end-user through the enum's value.

## Classes & structs

- Always use const whenever the function doesn't modify any data.

- Member functions are named "lowerCamelCase".

- For classes, all members have to be private, unless you want to allow getting and setting without creating those functions. Structs can have private data too; but most of the time structs should act as a simple 'class' with default constructors/destructors.

- Either create a copy, move, destruct and constructor yourself, mark them for delete or assign them to the default.

  - ```cpp
    T(const T&) = delete;
    T(T&&) = default;
    ```

- When adding a function to a class mark them explicit if you imagine them being misused.

- Mark a function static if it doesn't use any of the struct/class's data.

- Use initializer lists and default values over assignment.

- When declaring a virtual function, mark the function `final` when it should be the last override, mark it `virtual` when it can have an override, mark it `override` when it inherits it. 

- Avoid friend class if possible and only use protected functions/fields when you use a friend. 

## Template functions

- Avoid code duplication with and through templates
- Ensure template functions / classes are "safe" through type checking if possible (static_assert).
- Should be C++ standard, so no template functions in template classes or explicit template function declarations.

## Functions & variables

- Always pass the value by `const T&` if the value isn't easily copyable (Strings, Arrays, etc.), pass them by const if they aren't going to change.
- Make sure to avoid unnecessary copies, by using references whenever possible (exception being arithmetic types like integers, floats and pointers).
- Whenever a value can be constant, make it const.

## Syntax & auto

- Avoid long unnecessary statements like `std::vector<MyLongType>::iterator` by declaring them as a typedef (`using`) or by using `auto`.
- Avoid using `auto` if it takes away from the readability (like using auto for `Array<u32>`), as it is harder to understand what types are being passed around.
- Ifs/else ifs/else that only have one expression don't require curly braces, but if the expression becomes too complex they do.
- Avoid long lines of operations and make sure to leave whitespace in your code for readability.
- Use `T(...)` instead of `(T)...` for readability, whenever possible.
- Prefer arithmetic operators over ternary operators (to avoid unnecessary branching).
  - Use "pickIfTrue" for arithmetic types, like pointers and data types. `pickIfTrue(ifFalse, ifTrue, myBool)` instead of `myBool ? ifFalse : ifTrue`. This uses `ifFalse * (1 - myBool) + isTrue * myBool` instead of a branch.