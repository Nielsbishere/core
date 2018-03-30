#include <template/platformdefines.h>

#ifdef __ANDROID__

#include "window/windowmanager.h"
#include <android_native_app_glue.h>
#include <GLES/gl.h>
#include <utils/timer.h>
using namespace oi::wc;

void WindowManager::waitAll() {
	
	if(getWindows() != 1)
		Log::throwError<WindowManager, 0x0>("Only one window supported for Android");
	
	initAll();
	
	Window *w = operator[](0);
	struct android_app *state = (struct android_app*) w->info.handle;
	
	while(getWindows() != 0){
		
		int ident, events;
		struct android_poll_source *source;
		
		while((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0){
			
			if(source != nullptr)
				source->process(state, source);
			
		}
		
		Vec3 c = { 0.25, 0.5, 1 };
		c *= std::sin(Timer::getGlobalTimer().getDuration()) * 0.5 + 0.5;
		glClearColor(c.x, c.y, c.z, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		updateAll();
		
		if(state->destroyRequested != 0){
			Log::println("Destroy requested");
			remove(w);
		}
	}

}


#endif