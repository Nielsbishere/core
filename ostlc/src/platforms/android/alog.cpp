#include "template/platformdefines.h"

#ifdef __ANDROID__

#include "utils/log.h"
#include <android/log.h>
using namespace oi;

void printstr(String str){
	__android_log_print(ANDROID_LOG_INFO, "Log::print", "%s", str.toCString());
}

void printwar(String str){
	__android_log_print(ANDROID_LOG_WARN, "Log::warn", "%s", str.toCString());
}

void printerr(String str){
	__android_log_print(ANDROID_LOG_ERROR, "Log::error", "%s", str.toCString());
}

LogCallback Log::errorc = printerr, Log::warningc = printwar, Log::printc = printstr;

#endif