#include "utils/log.h"
using namespace oi;

bool Log::warn(String what) {
	warningc(what + String::lineEnd());
	return false;
}

bool Log::error(String what) {
	errorc(what + String::lineEnd());
	return false;
}

void Log::print(String what) {
	printc(what);
}

void Log::println(String what) {
	printc(what + String::lineEnd());
}

void Log::print(String what, LogLevel level) {
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