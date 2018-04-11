#ifdef __VULKAN__

#include "graphics/gl/vulkan.h"
#include "graphics/graphics.h"
#include "graphics/texture.h"
#include <window/window.h>

#include <cstring>

#undef min
#undef max

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

Graphics::~Graphics(){
	
	if(initialized){
		
		VkGraphics &graphics = *(VkGraphics*) platformData;

		destroySurface();
		vkDestroyDevice(graphics.device, allocator);
		vkDestroyInstance(graphics.instance, allocator);
		
		Log::println("Successfully destroyed Vulkan instance and device");
	}
	
}

void Graphics::init(Window *w, u32 buffering){
	
	this->buffering = buffering;
	
	if(sizeof(VkGraphics) > Graphics::platformSize)
		Log::throwError<Graphics, 0x0>("Graphics struct can't contain VkGraphics");
	
	VkGraphics &graphics = *(VkGraphics*) platformData;
	std::memset(&graphics, 0, sizeof(VkGraphics));

	//Get extensions and layers

	uint32_t layerCount, extensionCount;
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

	for (uint32_t i = 0; i < layerCount; ++i)
		if (String(layers[i].layerName) == "VK_LAYER_LUNARG_standard_validation") {
			clayers.push_back("VK_LAYER_LUNARG_standard_validation");
			break;
		}

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
	instanceInfo.enabledLayerCount = (uint32_t) clayers.size();
	instanceInfo.ppEnabledLayerNames = clayers.data();
	instanceInfo.enabledExtensionCount = (uint32_t) cextensions.size();
	instanceInfo.ppEnabledExtensionNames = cextensions.data();
	
	//Create instance
	
	vkCheck<0x1>(vkCreateInstance(&instanceInfo, allocator, &graphics.instance), "Couldn't obtain Vulkan instance");
	initialized = true;
	
	Log::println("Successfully initialized Graphics with Vulkan context");
	
	//Show all layers and extensions
	
	#ifdef __DEBUG__
	
	Log::println(String("Layers: ") + layerCount);
	for(uint32_t i = 0; i < layerCount; ++i){
		VkLayerProperties &l = layers[i];
		Log::println(String("Layer #") + i + ": " + l.layerName + " v" + l.specVersion + "-" + l.implementationVersion + ": " + l.description);
	}
	
	Log::println(String("Extensions: ") + extensionCount);
	for(uint32_t i = 0; i < extensionCount; ++i){
		VkExtensionProperties &e = extensions[i];
		Log::println(String("Extension #") + i + ": " + e.extensionName + " v" + e.specVersion);
	}
	
	#endif

	delete[] layers;
	delete[] extensions;
	
	//Get all devices
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, nullptr);
	
	VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, devices);
	
	#ifdef __DEBUG__
	Log::println(String("Devices: ") + deviceCount);
	#endif
	
	VkPhysicalDeviceProperties *properties = new VkPhysicalDeviceProperties[deviceCount];
	
	for(uint32_t i = 0; i < deviceCount; ++i){
		
		vkGetPhysicalDeviceProperties(devices[i], properties + i);
		
		#ifdef __DEBUG__
		Log::println(String("Device #") + i + ": " + properties[i].deviceName);
		#endif
		
	}
	
	VkPhysicalDevice *gpu = devices;
	
	bool foundDiscrete = false;
	
	for(uint32_t i = 0; i < deviceCount; ++i)
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
	
	graphics.pdevice = *gpu;
	
	delete[] properties;
	
	//Setup device
	
	VkDeviceCreateInfo deviceInfo;
	std::memset(&deviceInfo, 0, sizeof(deviceInfo));
	
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = (uint32_t) dlayers.size();
	deviceInfo.ppEnabledLayerNames = dlayers.data();
	deviceInfo.enabledExtensionCount = (uint32_t) dextensions.size();
	deviceInfo.ppEnabledExtensionNames = dextensions.data();
	deviceInfo.pEnabledFeatures = nullptr;
	

	uint32_t familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(graphics.pdevice, &familyCount, nullptr);

	VkQueueFamilyProperties *families = new VkQueueFamilyProperties[familyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(graphics.pdevice, &familyCount, families);


	float queuePriorities[] = { 1.f };
	constexpr uint32_t queueCount = (uint32_t)(sizeof(queuePriorities) / sizeof(float));
	VkDeviceQueueCreateInfo queues[queueCount];
	std::memset(queues, 0, sizeof(queues));
	
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueCount = queueCount;
	queues[0].pQueuePriorities = queuePriorities;
	
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queues;
	
	vkCheck<0x2>(vkCreateDevice(*gpu, &deviceInfo, allocator, &graphics.device), "Couldn't obtain device");
	
	Log::println("Successfully created device");
	delete[] families;
}


void Graphics::initSurface(Window *w){
	
	VkGraphics &graphics = *(VkGraphics*) platformData;
	
	//Enable extension
	
	vkExtension(vkGetPhysicalDeviceSurfaceFormatsKHR);
	
	//Setup device surface (Uses our custom CMake defines to make this 'cross platform')
	__VK_SURFACE_TYPE__ surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = __VK_SURFACE_STYPE__;
	
	void *platformBegin = &surfaceInfo.__VK_SURFACE_HANDLE__;			//This is the start where platform dependent data starts
	memcpy(platformBegin, w->getSurfaceData(), w->getSurfaceSize());	//Memcpy the bytes from the window's representation of the surface
	
	vkCheck<0x3>(__VK_SURFACE_CREATE__(graphics.instance, &surfaceInfo, NULL, &graphics.surface), "Couldn't obtain surface");
	
	//Check if the surface is supported

	VkBool32 supported = false;

	if(!vkCheck<0x4>(vkGetPhysicalDeviceSurfaceSupportKHR(graphics.pdevice, 0, graphics.surface, &supported), "Surface wasn't supported") || !supported)
		Log::throwError<Graphics, 0x5>("Surface wasn't supported");

	//Get the format we should display
	
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(graphics.pdevice, graphics.surface, &formatCount, NULL);
	
	if(formatCount == 0)
		Log::throwError<Graphics, 0x6>("Couldn't get surface format");
	
	VkSurfaceFormatKHR *formats = new VkSurfaceFormatKHR[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(graphics.pdevice, graphics.surface, &formatCount, formats);
	
	VkFormat colorFormat = formats[0].format;
	VkColorSpaceKHR colorSpace = formats[0].colorSpace;
	
	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	
	delete[] formats;
	
	graphics.colorFormat = colorFormat;
	graphics.colorSpace = colorSpace;
	
	TextureFormat format = VkTextureFormat::find(colorFormat).getName();
	
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics.pdevice, graphics.surface, &capabilities);
	
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
		
		uint32_t modeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics.pdevice, graphics.surface, &modeCount, nullptr);
		
		VkPresentModeKHR *modes = new VkPresentModeKHR[modeCount];
		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics.pdevice, graphics.surface, &modeCount, modes);
		
			for(uint32_t i = 0; i < modeCount; ++i)
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
	swapchainInfo.imageFormat = graphics.colorFormat;
	swapchainInfo.imageColorSpace = graphics.colorSpace;
	swapchainInfo.surface = graphics.surface;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;		//TODO: Rotation?
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = mode;
	
	vkCheck<0x9>(vkCreateSwapchainKHR(graphics.device, &swapchainInfo, allocator, &graphics.swapchain), "Couldn't create swapchain");
	
	vkGetSwapchainImagesKHR(graphics.device, graphics.swapchain, &buffering, nullptr);

	Log::println(String("Successfully created swapchain (with buffering option ") + buffering + ")");
	
	//Create the swapchain images
	
	std::vector<VkImage> swapchainImages = std::vector<VkImage>(buffering);
	vkGetSwapchainImagesKHR(graphics.device, graphics.swapchain, &buffering, swapchainImages.data());

	std::vector<Texture*> &textures = graphics.swapchainTextures = std::vector<Texture*>(buffering);
	
	for (u32 i = 0; i < buffering; ++i) {
		
		Texture *tex = textures[i] = new Texture(size, format, TextureUsage::Render_target);
		VkTexture &vkTex = *(VkTexture*) tex->platformData;
		vkTex.image = swapchainImages[i];
		
		if(!tex->init(this, false))
			Log::throwError<Graphics, 0xA>("Couldn't initialize swapchain image view");
	}

	//Create depth buffer

	vkGetPhysicalDeviceMemoryProperties(graphics.pdevice, &graphics.pmemory);
	graphics.swapchainDepth = create(size, TextureFormat::Depth, TextureUsage::Render_depth);

	Log::println("Successfully created image views of the swapchain");

	//TODO: Create 'FBO' from this
}

void Graphics::destroySurface(){
	
	VkGraphics &graphics = *(VkGraphics*) platformData;
	
	for (u32 i = 0; i < buffering; ++i)
		delete graphics.swapchainTextures[i];
	
	delete graphics.swapchainDepth;
	vkDestroySurfaceKHR(graphics.instance, graphics.surface, allocator);
	vkDestroySwapchainKHR(graphics.device, graphics.swapchain, allocator);
	
	Log::println("Successfully destroyed surface");
}

void Graphics::clear(Vec4f color) {
	//TODO: Clear screen
}

void Graphics::swapBuffers(){
	//TODO: Swap buffers
}

Texture *Graphics::create(Vec2u res, TextureFormat tf, TextureUsage usage) {

	Texture *tex = new Texture(res, tf, usage);

	if (!tex->init(this))
		return (Texture*)Log::throwError<Graphics, 0xB>("Couldn't create texture");

	return tex;
}

const char *Graphics::getShaderExtension() { return "spv"; }

#endif