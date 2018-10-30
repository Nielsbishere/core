#pragma once

#include <typeinfo>
#include "types/string.h"
#undef ERROR

namespace oi {

	typedef void (*LogCallback)(String str);

	enum class LogLevel {
		PRINT, WARN, ERROR, FATAL
	};

	class Log {

	public:

		static bool warn(String what);
		static bool error(String what);
		static void print(String what);
		static void println(String what);

		static void print(String what, LogLevel level);

		template<class T, u32 errorId>
		static bool throwError(String what) {
			String errorCode = String("Error [") + String::toHex(errorId) + " " + typeid(T).name() + "]: " + what;
			error(errorCode);
			throw std::runtime_error(errorCode.toCString());
			return false;
		}

		static void setCallback(LogCallback callback, LogLevel ll = LogLevel::PRINT);

	private:

		static LogCallback errorc, warningc, printc;
	};

}