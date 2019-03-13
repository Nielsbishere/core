#include "window/window.h"
#include "graphics/vulkan.h"
#include "graphics/graphics.h"
#include "windows/windows.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

void Graphics::setupSurface(Window *w) {

	//Setup device surface

	VkWin32SurfaceCreateInfoKHR surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

	surfaceInfo.hinstance = w->getExtension().instance;
	surfaceInfo.hwnd = w->getExtension().window;

	vkCheck<0xE>(vkCreateWin32SurfaceKHR(ext->instance, &surfaceInfo, vkAllocator, &ext->surface), "Couldn't obtain surface");

}

void GraphicsExt::getDebugInstanceParams(std::vector<const char*> &layers, std::vector<const char*> &extensions) {
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
	extensions.push_back("VK_EXT_debug_utils");
	extensions.push_back("VK_EXT_debug_report");
}

void GraphicsExt::getPlatformInstanceParams(std::vector<const char*>&, std::vector<const char*> &extensions) {
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void GraphicsExt::initializePlatform() {
	#if defined(__DEBUG__)
		debugNames = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
	#endif
}