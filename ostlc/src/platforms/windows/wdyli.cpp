#include "template/platformdefines.h"

#ifdef __WINDOWS__

#include "utils/dyli.h"
#include "utils/log.h"
using namespace oi;

#define dll (*(HINSTANCE*) platformData)

DyLi::~DyLi() {

	if (isValid())
		FreeLibrary(dll);
}

void *DyLi::getFuncPtr(String name) const {
	return (void*) GetProcAddress(dll, name.toCString());
}

void DyLi::init(String path) {

	dll = LoadLibrary((path + ".dll").toCString());

	if (!isValid())
		Log::throwError<DyLi, 0x0>(String("Couldn't load DLL with path \"") + path + "\"");
}

bool DyLi::isValid() const {
	return dll != NULL;
}

#endif