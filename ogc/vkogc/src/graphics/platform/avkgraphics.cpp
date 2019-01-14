#if defined(__ANDROID__)
#include "window/window.h"
#include "platforms/android.h"
#include "graphics/vulkan.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

void Graphics::setupSurface(Window *w) {

	//Setup device surface (Uses our custom CMake defines to make this 'cross platform')
	VkAndroidSurfaceCreateInfoKHR surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	
	surfaceInfo.window = w->getExtension().app->window;

	vkCheck<0xE>(vkCreateAndroidSurfaceKHR(ext->instance, &surfaceInfo, vkAllocator, &ext->surface), "Couldn't obtain surface");

}

#endif