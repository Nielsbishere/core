#pragma once

#include <typeinfo>
#include "types/string.h"

#undef ERROR

namespace oi {

	typedef void (*LogCallback)(const String &str);

	enum class LogLevel {
		PRINT, WARN, ERROR, FATAL
	};

	class Log {

	public:

		static bool warn(const String &what);
		static bool error(const String &what);
		static void print(const String &what);
		static void println(const String &what);

		static void print(const String &what, LogLevel level);

		template<class T, u32 errorId>
		static bool throwError(const String &what) {
			String errorCode = String("Error [") + String::toHex(errorId) + " " + typeid(T).name() + "]: " + what;
			error(errorCode);
			throw std::runtime_error(errorCode.begin());
			return false;
		}

		static void setCallback(LogCallback callback, LogLevel ll = LogLevel::PRINT);

	protected:

		static bool setupExitHandler();
		static bool setupExitHandlerValue;

	private:

		static LogCallback errorc, warningc, printc;
	};

}