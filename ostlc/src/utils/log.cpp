#include "utils/log.h"
using namespace oi;

void Log::warn(String what) {
	warningc(what + String::lineEnd());
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