#include "Utils/Log.h"
using namespace oi;

void stdprint(OString what) {
	printf("%s", what.c_str());
}

LogCallback Log::errorc = stdprint, Log::warningc = stdprint, Log::printc = stdprint;

void Log::warn(OString what) {
	warningc(what + OString::lineEnd());
}

bool Log::error(OString what) {
	errorc(what + OString::lineEnd());
	return false;
}

void Log::print(OString what) {
	printc(what);
}

void Log::println(OString what) {
	printc(what + OString::lineEnd());
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