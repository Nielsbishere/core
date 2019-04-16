#include "utils/log.h"
#include <Windows.h>
using namespace oi;

void printstr(const String &str){
	printf("%s", str.begin());
}

void printerr(const String &str){
	OutputDebugStringA(str.begin());
	printstr(str);
}

LogCallback Log::errorc = printerr, Log::warningc = printstr, Log::printc = printstr;