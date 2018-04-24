#ifdef __VULKAN__

#include "graphics/gl/vulkan.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include "graphics/rendertarget.h"
#include "graphics/commandlist.h"
#include <window/window.h>

#include <cstring>

#undef min
#undef max

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

VkBool32 onDebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u64 object, size_t location, i32 messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData) {
	
	String prefix;
	LogLevel level = LogLevel::PRINT;
	
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		prefix = "Info: ";
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		prefix = "Warning: ";
		level = LogLevel::WARN;
	} else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		prefix = "Performance warning: ";
		level = LogLevel::WARN;
	} else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		prefix = "Error: ";
		level = LogLevel::ERROR;
	} else 
		prefix = "Debug: ";

	Log::print(prefix + pMessage, level);

	return VK_FALSE;
}

Graphics::~Graphics(){
	
	if(initialized){

		vkQueueWaitIdle(ext.queue);

		vkDestroyCommandPool(ext.device, ext.pool, allocator);
		vkDestroySemaphore(ext.device, ext.semaphore, allocator);
		vkDestroyFence(ext.device, ext.present, allocator);

		destroySurface();
		vkDestroyDevice(ext.device, allocator);

		#ifdef __DEBUG__

		vkExtension(vkDestroyDebugReportCallbackEXT);

		vkDestroyDebugReportCallbackEXT(ext.instance, ext.debugCallback, allocator);
		#endif

		vkDestroyInstance(ext.instance, allocator);
		
		Log::println("Successfully destroyed Vulkan instance and device");
	}
	
}

void Graphics::init(Window *w, u32 buffering){

	this->buffering = buffering;

	//Get extensions and layers

	u32 layerCount, extensionCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	VkLayerProperties *layers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, layers);

	VkExtensionProperties *extensions = new VkExtensionProperties[extensionCount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

	//Constants
	
	const u32 majorVersion = 1, minorVersion = 0, patchVersion = 0;					///Vulkan version
	
	std::vector<const char*> clayers, cextensions = std::vector<const char*>(2);	///Instance layers and extensions
	cextensions[0] = "VK_KHR_surface";
	cextensions[1] = __VK_SURFACE_EXT__;

	#ifdef __DEBUG__

	#ifdef __ANDROID__
	clayers = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
				"VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain", "VK_LAYER_GOOGLE_unique_objects" };
	#else 
	clayers.push_back("VK_LAYER_LUNARG_standard_validation");
	#endif

	cextensions.push_back("VK_EXT_debug_report");
	#endif
	
	std::vector<const char*> dlayers, dextensions(1);								///Device layers and extensions
	dextensions[0] = "VK_KHR_swapchain";
	
	//Set up the application
	
	VkApplicationInfo application;
	std::memset(&application, 0, sizeof(application));
	
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	
	application.pApplicationName = w->getInfo().getTitle().toCString();
	application.applicationVersion = w->getInfo().getVersion();
	
	application.pEngineName = "Osomi Graphics Core";
	application.engineVersion = 1;
	
	application.apiVersion = VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	
	//Set up the instance info
	
	VkInstanceCreateInfo instanceInfo;
	std::memset(&instanceInfo, 0, sizeof(instanceInfo));
	
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &application;
	instanceInfo.enabledLayerCount = (u32) clayers.size();
	instanceInfo.ppEnabledLayerNames = clayers.data();
	instanceInfo.enabledExtensionCount = (u32) cextensions.size();
	instanceInfo.ppEnabledExtensionNames = cextensions.data();
	
	//Create instance
	
	Log::println(String("Creating Vulkan instance with ") + (u32) cextensions.size() + " extensions & " + (u32) clayers.size() + " layers:");

	for (auto exten : cextensions)
		Log::println(String("Extension ") + exten);

	for (auto lay : clayers)
		Log::println(String("Layer ") + lay);

	vkCheck<0x1>(vkCreateInstance(&instanceInfo, allocator, &ext.instance), "Couldn't obtain Vulkan instance");
	initialized = true;
	
	Log::println("Successfully initialized Graphics with Vulkan context");


	#ifdef __DEBUG__

	//Debug callback

	VkDebugReportCallbackCreateInfoEXT callbackInfo;
	memset(&callbackInfo, 0, sizeof(callbackInfo));

	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;	//All but info
	callbackInfo.pfnCallback = onDebugReport;

	vkExtension(vkCreateDebugReportCallbackEXT);

	vkCheck<0x19>(vkCreateDebugReportCallbackEXT(ext.instance, &callbackInfo, allocator, &ext.debugCallback), "Couldn't create debug report callback");

	Log::println("Successfully created debug report callback");

	#endif

	//Show all layers and extensions
	
	#ifdef __DEBUG__
	
	Log::println(String("Layers: ") + layerCount);
	for(u32 i = 0; i < layerCount; ++i){
		VkLayerProperties &l = layers[i];
		Log::println(String("Layer #") + i + ": " + l.layerName + " v" + l.specVersion + "-" + l.implementationVersion + ": " + l.description);
	}
	
	Log::println(String("Extensions: ") + extensionCount);
	for(u32 i = 0; i < extensionCount; ++i){
		VkExtensionProperties &e = extensions[i];
		Log::println(String("Extension #") + i + ": " + e.extensionName + " v" + e.specVersion);
	}
	
	#endif

	delete[] layers;
	delete[] extensions;
	
	//Get all devices
	u32 deviceCount;
	vkEnumeratePhysicalDevices(ext.instance, &deviceCount, nullptr);
	
	VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(ext.instance, &deviceCount, devices);
	
	#ifdef __DEBUG__
	Log::println(String("Devices: ") + deviceCount);
	#endif
	
	VkPhysicalDeviceProperties *properties = new VkPhysicalDeviceProperties[deviceCount];
	
	for(u32 i = 0; i < deviceCount; ++i){
		
		vkGetPhysicalDeviceProperties(devices[i], properties + i);
		
		#ifdef __DEBUG__
		Log::println(String("Device #") + i + ": " + properties[i].deviceName);
		#endif
		
	}
	
	VkPhysicalDevice *gpu = devices;
	
	bool foundDiscrete = false;
	
	for(u32 i = 0; i < deviceCount; ++i)
		if(properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			
			#ifdef __DEBUG__
			Log::println(String("Found a discrete GPU (") + properties[i].deviceName + ")");
			#endif
			
			gpu = devices + i;
			foundDiscrete = true;
			break;
			
		}
		
	if(!foundDiscrete)
		Log::warn("Couldn't find a discrete GPU; so instead picked the first");
	
	ext.pdevice = *gpu;
	
	delete[] properties;
	
	//Setup device
	
	VkDeviceCreateInfo deviceInfo;
	std::memset(&deviceInfo, 0, sizeof(deviceInfo));
	
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = (u32) dlayers.size();
	deviceInfo.ppEnabledLayerNames = dlayers.data();
	deviceInfo.enabledExtensionCount = (u32) dextensions.size();
	deviceInfo.ppEnabledExtensionNames = dextensions.data();
	deviceInfo.pEnabledFeatures = nullptr;
	

	u32 familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(ext.pdevice, &familyCount, nullptr);

	VkQueueFamilyProperties *families = new VkQueueFamilyProperties[familyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(ext.pdevice, &familyCount, families);

	u32 queueFamilyIndex = u32_MAX;

	for (u32 i = 0; i < familyCount; ++i) {

		VkQueueFamilyProperties &fam = families[i];

		if (fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_GRAPHICS_BIT && fam.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			queueFamilyIndex = i;
			break;
		}

	}

	if (queueFamilyIndex == u32_MAX)
		Log::throwError<Graphics, 0x1F>("Couldn't intialize family queue");

	float queuePriorities[] = { 1.f };
	constexpr u32 queueCount = (u32)(sizeof(queuePriorities) / sizeof(float));
	VkDeviceQueueCreateInfo queues[queueCount];
	std::memset(queues, 0, sizeof(queues));
	
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueCount = queueCount;
	queues[0].pQueuePriorities = queuePriorities;
	
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queues;
	
	vkCheck<0x2>(vkCreateDevice(*gpu, &deviceInfo, allocator, &ext.device), "Couldn't obtain device");
	
	vkGetDeviceQueue(ext.device, queueFamilyIndex, 0, &ext.queue);

	Log::println("Successfully created device");
	delete[] families;

	//Create present fence

	VkFenceCreateInfo fenceInfo;
	memset(&fenceInfo, 0, sizeof(fenceInfo));

	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCheck<0xE>(vkCreateFence(ext.device, &fenceInfo, allocator, &ext.present), "Couldn't create the present fence");

	//Create semaphore

	VkSemaphoreCreateInfo semaphoreInfo;
	memset(&semaphoreInfo, 0, sizeof(semaphoreInfo));

	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCheck<0x17>(vkCreateSemaphore(ext.device, &semaphoreInfo, allocator, &ext.semaphore), "Couldn't create semaphore");


	//Create command pool

	VkCommandPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	vkCheck<0x16>(vkCreateCommandPool(ext.device, &poolInfo, allocator, &ext.pool), "Couldn't create command pool");

}


void Graphics::initSurface(Window *w){

	//Enable extension
	
	vkExtension(vkGetPhysicalDeviceSurfaceFormatsKHR);
	
	//Setup device surface (Uses our custom CMake defines to make this 'cross platform')
	__VK_SURFACE_TYPE__ surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = __VK_SURFACE_STYPE__;
	
	void *platformBegin = &surfaceInfo.__VK_SURFACE_HANDLE__;			//This is the start where platform dependent data starts
	memcpy(platformBegin, w->getSurfaceData(), w->getSurfaceSize());	//Memcpy the bytes from the window's representation of the surface
	
	vkCheck<0x3>(__VK_SURFACE_CREATE__(ext.instance, &surfaceInfo, allocator, &ext.surface), "Couldn't obtain surface");
	
	//Check if the surface is supported

	VkBool32 supported = false;

	if(!vkCheck<0x4>(vkGetPhysicalDeviceSurfaceSupportKHR(ext.pdevice, 0, ext.surface, &supported), "Surface wasn't supported") || !supported)
		Log::throwError<Graphics, 0x5>("Surface wasn't supported");

	//Get the format we should display
	
	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ext.pdevice, ext.surface, &formatCount, NULL);
	
	if(formatCount == 0)
		Log::throwError<Graphics, 0x6>("Couldn't get surface format");
	
	VkSurfaceFormatKHR *formats = new VkSurfaceFormatKHR[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(ext.pdevice, ext.surface, &formatCount, formats);
	
	VkFormat colorFormat = formats[0].format;
	VkColorSpaceKHR colorSpace = formats[0].colorSpace;
	
	delete[] formats;
	
	ext.colorFormat = colorFormat;
	ext.colorSpace = colorSpace;
	
	TextureFormat format = TextureFormatExt::find(colorFormat).getName();
	
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ext.pdevice, ext.surface, &capabilities);
	
	Vec2u size = { capabilities.currentExtent.width, capabilities.currentExtent.height };
	
	if(size == Vec2u::max())
		Log::throwError<Graphics, 0x7>("Size is undefined; this is not supported!");
	
	w->getInfo()._forceSize(size);
	
	Log::println("Successfully created surface");
	
	//Create swapchain
	
	VkSwapchainCreateInfoKHR swapchainInfo;
	memset(&swapchainInfo, 0, sizeof(swapchainInfo));
	
	//Try to support mailbox present mode if triple buffering is enabled
	
	if(buffering > capabilities.maxImageCount && capabilities.maxImageCount > 0) buffering = capabilities.maxImageCount;
	if(buffering < capabilities.minImageCount) buffering = capabilities.minImageCount;
	
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR desire = buffering == 1 ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_MAILBOX_KHR;
	
	if(buffering != 2){
		
		u32 modeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(ext.pdevice, ext.surface, &modeCount, nullptr);
		
		VkPresentModeKHR *modes = new VkPresentModeKHR[modeCount];
		vkGetPhysicalDeviceSurfacePresentModesKHR(ext.pdevice, ext.surface, &modeCount, modes);
		
			for(u32 i = 0; i < modeCount; ++i)
				if(modes[i] == desire){
					mode = modes[i];
					break;
				}
			
		delete[] modes;
	}
	
	if(buffering == 1 && mode != desire)
		Log::throwError<Graphics, 0x8>("Immediate presentMode is required for single buffering");
	
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.minImageCount = buffering;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.imageExtent.width = size.x;
	swapchainInfo.imageExtent.height = size.y;
	swapchainInfo.imageFormat = ext.colorFormat;
	swapchainInfo.imageColorSpace = ext.colorSpace;
	swapchainInfo.surface = ext.surface;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainInfo.compositeAlpha = (VkCompositeAlphaFlagBitsKHR) capabilities.supportedCompositeAlpha;
	swapchainInfo.presentMode = mode;

	vkCheck<0x9>(vkCreateSwapchainKHR(ext.device, &swapchainInfo, allocator, &ext.swapchain), "Couldn't create swapchain");
	
	vkGetSwapchainImagesKHR(ext.device, ext.swapchain, &buffering, nullptr);

	Log::println(String("Successfully created swapchain (with buffering option ") + buffering + ")");
	
	//Create the swapchain images
	
	std::vector<VkImage> swapchainImages = std::vector<VkImage>(buffering);
	vkGetSwapchainImagesKHR(ext.device, ext.swapchain, &buffering, swapchainImages.data());

	std::vector<Texture*> textures = std::vector<Texture*>(buffering);
	
	for (u32 i = 0; i < buffering; ++i) {
		
		Texture *tex = textures[i] = new Texture(TextureInfo(size, format, TextureUsage::Render_target));
		VkTexture &vkTex = tex->getExtension();
		vkTex.image = swapchainImages[i];
		
		if(!tex->init(this, false))
			Log::throwError<Graphics, 0xA>("Couldn't initialize swapchain image view");
	}

	//Create depth buffer

	vkGetPhysicalDeviceMemoryProperties(ext.pdevice, &ext.pmemory);
	Texture *depthBuffer = create(TextureInfo(size, TextureFormat::Depth, TextureUsage::Render_depth));

	Log::println("Successfully created image views of the swapchain");

	//Turn it into a RenderTarget aka 'Render pass'

	backBuffer = new RenderTarget(RenderTargetInfo(size, depthBuffer->getFormat(), { VkTextureFormat(colorFormat).getName() }, buffering), depthBuffer, textures);

	if(!backBuffer->init(this))
		Log::throwError<Graphics, 0xC>("Couldn't initialize backBuffer (render target)");

	Log::println("Successfully created backBuffer");
}

void Graphics::destroySurface(){

	vkQueueWaitIdle(ext.queue);

	vkDestroySwapchainKHR(ext.device, ext.swapchain, allocator);
	delete backBuffer;
	vkDestroySurfaceKHR(ext.instance, ext.surface, allocator);
	
	Log::println("Successfully destroyed surface");
}

void Graphics::begin() {

	vkCheck<0x10>(vkAcquireNextImageKHR(ext.device, ext.swapchain, u64_MAX, VK_NULL_HANDLE, ext.present, &ext.current), "Couldn't acquire next image");
	vkCheck<0x11>(vkWaitForFences(ext.device, 1, &ext.present, VK_TRUE, u64_MAX), "Couldn't wait for fences");
	vkCheck<0x12>(vkResetFences(ext.device, 1, &ext.present), "Couldn't reset fences");
	vkCheck<0x13>(vkQueueWaitIdle(ext.queue), "Couldn't wait idle");

}

void Graphics::end() {

	//Submit commands

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));

	std::vector<VkCommandBuffer> commandBuffer(commandList.size());
	for (u32 i = 0; i < (u32) commandBuffer.size(); ++i)
		commandBuffer[i] = commandList[i]->ext.cmd;

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (u32) commandList.size();
	submitInfo.pCommandBuffers = commandBuffer.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &ext.semaphore;

	vkCheck<0x18>(vkQueueSubmit(ext.queue, 1, &submitInfo, nullptr), "Couldn't submit queue");

	//Present it

	VkResult result = VK_SUCCESS;

	VkPresentInfoKHR presentInfo;
	memset(&presentInfo, 0, sizeof(presentInfo));

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &ext.swapchain;
	presentInfo.pResults = &result;
	presentInfo.pImageIndices = &ext.current;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &ext.semaphore;

	vkCheck<0xF>(vkQueuePresentKHR(ext.queue, &presentInfo), "Couldn't present image");
	vkCheck<0x14>(result, "Couldn't present image");

}

CommandList *Graphics::create(CommandListInfo info) {

	CommandList *cl = new CommandList(info);

	cl->getExtension().pool = ext.pool;

	if (!cl->init(this))
		Log::throwError<Graphics, 0x15>("Couldn't create command list");

	commandList.push_back(cl);

	return cl;
}

#endif