#include <main.h>

void android_main(struct android_app *state){
	Application::instantiate((void*)state);
}