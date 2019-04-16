#include "window/window.h"
#include "viewport/windowviewport.hpp"
#include "android/android.h"
#include "graphics/vulkan.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

void Graphics::setupSurface(Window *w) {

	if (WindowViewport *wv = w->getViewport<WindowViewport>()) {

		//Setup device surface

		VkAndroidSurfaceCreateInfoKHR surfaceInfo;
		memset(&surfaceInfo, 0, sizeof(surfaceInfo));
		surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		
		surfaceInfo.window = wv->getExt()->app->window;

		vkCheck<0xE>(vkCreateAndroidSurfaceKHR(ext->instance, &surfaceInfo, vkAllocator, &ext->surface), "Couldn't obtain surface");

	}

}

void GraphicsExt::getDebugInstanceParams(List<String> &layers, List<String> &extensions) {
	layers.pushBackUnique("VK_LAYER_GOOGLE_threading");
	layers.pushBackUnique("VK_LAYER_LUNARG_parameter_validation");
	layers.pushBackUnique("VK_LAYER_LUNARG_object_tracker");
	layers.pushBackUnique("VK_LAYER_LUNARG_core_validation");
	layers.pushBackUnique("VK_LAYER_GOOGLE_unique_objects");
	layers.pushBackUnique("VK_LAYER_GOOGLE_threading");
	extensions.pushBackUnique("VK_EXT_debug_report");
}

void GraphicsExt::getPlatformInstanceParams(List<String>&, List<String> &extensions) {
	extensions.pushBackUnique(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
}

void GraphicsExt::initializePlatform() { }