#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "window/windowmanager.h"
#include <android_native_app_glue.h>
using namespace oi::wc;

void WindowManager::waitAll() {
	
	initAll();
	
	while(getWindows() != 0){
		
		int ident, events;
		struct android_poll_source *source;
		
		while((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0){
			
			if(source != nullptr){
				struct android_app *state = source->app;
				source->process(state, source);
			}
			
		}
		
		updateAll();
	}

}


#endif