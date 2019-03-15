#include "utils/log.h"
using namespace oi;

bool Log::warn(const String &what) {
	#ifndef NO_LOG
	warningc(what + String::lineEnd());
	#endif
	return false;
}

bool Log::error(const String &what) {
	#ifndef NO_LOG
	errorc(what + String::lineEnd());
	#endif
	return false;
}

void Log::print(const String &what) {
	#ifndef NO_LOG
	printc(what);
	#endif
}

void Log::println(const String &what) {
	#ifndef NO_LOG
	printc(what + String::lineEnd());
	#endif
}

void Log::print(const String &what, LogLevel level) {
	if (level == LogLevel::ERROR) Log::error(what);
	else if (level == LogLevel::FATAL) Log::throwError<Log, 0x0>(what);
	else if (level == LogLevel::WARN) Log::warn(what);
	else Log::println(what);
}

void Log::setCallback(LogCallback callback, LogLevel ll) {
	switch (ll) {
	case LogLevel::ERROR:
		errorc = callback;
		break;
	case LogLevel::WARN:
		warningc = callback;
		break;
	default:
		printc = callback;
		break;
	}
}