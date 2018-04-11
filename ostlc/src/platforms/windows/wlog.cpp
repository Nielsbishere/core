#ifdef __WINDOWS__

#include "utils/log.h"
#include <Windows.h>
using namespace oi;

void printstr(String str){
	printf("%s", str.toCString());
}

void printerr(String str){
	OutputDebugString(str.toCString());
	printstr(str);
}

LogCallback Log::errorc = printerr, Log::warningc = printstr, Log::printc = printstr;

#endif