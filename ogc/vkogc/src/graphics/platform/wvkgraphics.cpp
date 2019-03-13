#if defined(__WINDOWS__)
#include "window/window.h"
#include "graphics/vulkan.h"
#include "graphics/graphics.h"
#include "windows/windows.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

void Graphics::setupSurface(Window *w) {

	//Setup device surface (Uses our custom CMake defines to make this 'cross platform')
	VkWin32SurfaceCreateInfoKHR surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

	surfaceInfo.hinstance = w->getExtension().instance;
	surfaceInfo.hwnd = w->getExtension().window;

	vkCheck<0xE>(vkCreateWin32SurfaceKHR(ext->instance, &surfaceInfo, vkAllocator, &ext->surface), "Couldn't obtain surface");

}

#endif