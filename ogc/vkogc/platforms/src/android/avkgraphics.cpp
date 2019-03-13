#include "window/window.h"
#include "android/android.h"
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

void GraphicsExt::getDebugInstanceParams(std::vector<const char*> &layers, std::vector<const char*> &extensions) {

	layers.insert(layers.end(), {
		"VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects"
	});

	extensions.push_back("VK_EXT_debug_report");

}

void GraphicsExt::getPlatformInstanceParams(std::vector<const char*>&, std::vector<const char*> &extensions) {
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
}

void GraphicsExt::initializePlatform() {}