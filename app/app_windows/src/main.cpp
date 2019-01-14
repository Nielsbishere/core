#include <main.h>

#ifdef __NO_CONSOLE__
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main() {
	Application::instantiate();
	return 0;
}