#pragma once

#include "Types/OString.h"

namespace oi {

	typedef void (*LogCallback)(OString str);

	enum class LogLevel {
		PRINT, WARN, ERROR
	};

	class Log {

	public:

		static void warn(OString what);
		static bool error(OString what);
		static void print(OString what);
		static void println(OString what);

		template<class T, u32 errorId>
		static bool throwError(OString what) {
			OString errorCode = OString("Error [") + OString::toHex(errorId) + " " + typeid(T).name() + "]: " + what;
			error(errorCode);
			throw(std::exception(errorCode.c_str()));
			return false;
		}

		static void setCallback(LogCallback callback, LogLevel ll = LogLevel::PRINT);

	private:

		static LogCallback errorc, warningc, printc;
	};

}