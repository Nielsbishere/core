#include "utils/log.h"
#include <Windows.h>
using namespace oi;

void printstr(String str){
	printf("%s", str.begin());
}

void printerr(String str){
	OutputDebugStringA(str.begin());
	printstr(str);
}

LogCallback Log::errorc = printerr, Log::warningc = printstr, Log::printc = printstr;