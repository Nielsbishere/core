#ifdef __VULKAN__

#include <cstring>
#include "window/window.h"
#include "graphics/graphics.h"
#include "graphics/interface/graphicsinterface.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/gpubuffer.h"


#undef min
#undef max
#undef ERROR

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

VkBool32 onDebugReport(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, u64, size_t, i32, const char*, const char *pMessage, void*) {
	
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
	
	destroy(ext.stagingCmdList);

	if(initialized){

		destroy(backBuffer);

		for (auto &a : objects)
			for (u32 i = (u32) a.second.size() - 1; i != u32_MAX; --i) {
				Log::warn(String("Left over object ") + a.second[i]->getName() + " (" + a.second[i]->getTypeName() + ") #" + i + " and refCount " + a.second[i]->refCount);
				destroyObject(a.second[i]);
			}

		objects.clear();

		vkDestroyCommandPool(ext.device, ext.pool, vkAllocator);

		destroySurface();

		Log::println("HI vkDestroyDevice");

		vkDestroyDevice(ext.device, vkAllocator);

		#ifdef __DEBUG__

			vkExtension(vkDestroyDebugReportCallbackEXT);

			vkDestroyDebugReportCallbackEXT(ext.instance, ext.debugCallback, vkAllocator);

		#endif

		vkDestroyInstance(ext.instance, vkAllocator);
		
		Log::println("Successfully destroyed Vulkan instance and device");
	}
	
}

void Graphics::init(Window *w){

	this->buffering = 3;				//Assume triple buffering

	//Get extensions and layers

	u32 layerCount = 0, extensionCount = 0;
	vkCheck<0x20>(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "Couldn't enumerate layers");
	vkCheck<0x21>(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr), "Couldn't enumerate extensions");

	VkLayerProperties *layers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, layers);

	VkExtensionProperties *extensions = new VkExtensionProperties[extensionCount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

	#ifdef __DEBUG__

		Log::println("Starting graphics...");
		Log::println("Supported layers:");
		
		for(u32 i = 0; i < layerCount; ++i)
			Log::println(String("\t") + layers[i].layerName);
		
		Log::println("Supported extensions:");
		
		for (u32 i = 0; i < extensionCount; ++i)
			Log::println(String("\t") + extensions[i].extensionName);
	
	#endif
	
	//Constants
	
	const u32 majorVersion = 1, minorVersion = 0, patchVersion = 0;					///Vulkan version
	
	std::vector<const char*> clayers, cextensions = std::vector<const char*>(2);	///Instance layers and extensions
	cextensions[0] = "VK_KHR_surface";
	cextensions[1] = __VK_SURFACE_EXT__;

	#ifdef __DEBUG__

		#ifdef __ANDROID__
			clayers = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
					"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" };
		#else 
			clayers.push_back("VK_LAYER_LUNARG_standard_validation");
			cextensions.push_back("VK_EXT_debug_utils");
		#endif

		cextensions.push_back("VK_EXT_debug_report");
	#endif

	bool supported = false;

	for (VkExtensionProperties *extension = extensions; extension != extensions + extensionCount; ++extension) {
		if (String(extension->extensionName) == VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME){
			cextensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			supported = true;
		}
	}

	if(!supported)
		Log::throwError<VkGraphics, 0x24>("Vulkan driver not supported; PhysicalDeviceProperties2 required");

	std::vector<const char*> dlayers, dextensions(2);								///Device layers and extensions
	dextensions[0] = "VK_KHR_swapchain";
	dextensions[1] = "VK_KHR_shader_draw_parameters";

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

	#ifdef __DEBUG__

	Log::println("\tExtensions:");

	for (auto exten : cextensions)
		Log::println(String("\t\t") + exten);

	Log::println("\tLayers:");

	for (auto lay : clayers)
		Log::println(String("\t\t") + lay);

	#endif

	vkCheck<0xA>(vkCreateInstance(&instanceInfo, vkAllocator, &ext.instance), "Couldn't obtain Vulkan instance");
	initialized = true;
	
	Log::println("Successfully initialized Graphics with Vulkan context");


	#ifdef __DEBUG__

		//Debug callback

		VkDebugReportCallbackCreateInfoEXT callbackInfo;
		memset(&callbackInfo, 0, sizeof(callbackInfo));

		callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;	//All but info
		callbackInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT) onDebugReport;	//TODO: Warning on some devices; not same type?

		vkExtension(vkCreateDebugReportCallbackEXT);

		vkCheck<0xB>(vkCreateDebugReportCallbackEXT(ext.instance, &callbackInfo, vkAllocator, &ext.debugCallback), "Couldn't create debug report callback");

		Log::println("Successfully created debug report callback");

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
	
	VkPhysicalDeviceProperties2 *properties = new VkPhysicalDeviceProperties2[deviceCount];
	memset(properties, 0, sizeof(VkPhysicalDeviceProperties2) * deviceCount);
	
	for(u32 i = 0; i < deviceCount; ++i){
		
		properties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		vkGetPhysicalDeviceProperties2(devices[i], properties + i);
		
		#ifdef __DEBUG__
			Log::println(String("Device #") + i + ": " + properties[i].properties.deviceName);
		#endif
		
	}
	
	VkPhysicalDevice *gpu = devices;
	
	bool foundDiscrete = false;
	
	for(u32 i = 0; i < deviceCount; ++i)
		if(properties[i].properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			
			#ifdef __DEBUG__
				Log::println(String("Found a discrete GPU (") + properties[i].properties.deviceName + ")");
			#endif
			
			gpu = devices + i;
			foundDiscrete = true;
			break;
			
		}
		
	if(!foundDiscrete)
		Log::warn("Couldn't find a discrete GPU; so instead picked the first");
	
	ext.pdevice = *gpu;
	
	delete[] properties;

	vkGetPhysicalDeviceFeatures(ext.pdevice, &ext.pfeatures);

	memset(&ext.pproperties, 0, sizeof(ext.pproperties));
	ext.pproperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	vkGetPhysicalDeviceProperties2(ext.pdevice, &ext.pproperties);

	//Query device extensions

	layerCount = extensionCount = 0;
	vkCheck<0x22>(vkEnumerateDeviceLayerProperties(ext.pdevice, &layerCount, nullptr), "Couldn't enumerate device layers");
	vkCheck<0x23>(vkEnumerateDeviceExtensionProperties(ext.pdevice, nullptr, &extensionCount, nullptr), "Couldn't enumerate device extensions");

	layers = new VkLayerProperties[layerCount];
	vkEnumerateDeviceLayerProperties(ext.pdevice, &layerCount, layers);

	extensions = new VkExtensionProperties[extensionCount];
	vkEnumerateDeviceExtensionProperties(ext.pdevice, nullptr, &extensionCount, extensions);

	#ifdef __DEBUG__

		Log::println("Supported device layers:");
		
		for(u32 i = 0; i < layerCount; ++i)
			Log::println(String("\t") + layers[i].layerName);
		
		Log::println("Supported device extensions:");
		
		for (u32 i = 0; i < extensionCount; ++i)
			Log::println(String("\t") + extensions[i].extensionName);
	
	#endif

	for(VkExtensionProperties *extension = extensions; extension < extensions + extensionCount; ++extension){

		#ifdef __VR__
		if (String(extension->extensionName) == VK_KHR_MULTIVIEW_EXTENSION_NAME) {
			features[GraphicsFeature::VR] = true;
			dextensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
		}
		else
		#endif

		#ifdef __RAYTRACING__
			if(String(extension->extensionName) == VK_NV_RAY_TRACING_EXTENSION_NAME) {
				features[GraphicsFeature::Raytracing] = true;
				dextensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
			}
		#endif

	}

	delete[] layers;
	delete[] extensions;

	//Setup device
	
	VkDeviceCreateInfo deviceInfo;
	std::memset(&deviceInfo, 0, sizeof(deviceInfo));
	
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = (u32) dlayers.size();
	deviceInfo.ppEnabledLayerNames = dlayers.data();
	deviceInfo.enabledExtensionCount = (u32) dextensions.size();
	deviceInfo.ppEnabledExtensionNames = dextensions.data();
	deviceInfo.pEnabledFeatures = &ext.pfeatures;

	u32 familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(ext.pdevice, &familyCount, nullptr);

	VkQueueFamilyProperties *families = new VkQueueFamilyProperties[familyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(ext.pdevice, &familyCount, families);

	for (u32 i = 0; i < familyCount; ++i) {

		VkQueueFamilyProperties &fam = families[i];

		if (fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_GRAPHICS_BIT && fam.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			ext.queueFamilyIndex = i;
			break;
		}

	}

	if (ext.queueFamilyIndex == u32_MAX)
		Log::throwError<VkGraphics, 0x0>("Couldn't intialize family queue");

	float queuePriorities[] = { 1.f };
	constexpr u32 queueCount = (u32)(sizeof(queuePriorities) / sizeof(float));
	VkDeviceQueueCreateInfo queues[queueCount];
	std::memset(queues, 0, sizeof(queues));
	
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueCount = queueCount;
	queues[0].pQueuePriorities = queuePriorities;
	
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queues;
	
	Log::println(String("Creating Vulkan device with ") + (u32) dextensions.size() + " extensions & " + (u32) dlayers.size() + " layers:");

	#ifdef __DEBUG__

	Log::println("\tExtensions:");

	for (auto exten : dextensions)
		Log::println(String("\t\t") + exten);

	Log::println("\tLayers:");

	for (auto lay : dlayers)
		Log::println(String("\t\t") + lay);

	#endif

	vkCheck<0xC>(vkCreateDevice(*gpu, &deviceInfo, vkAllocator, &ext.device), "Couldn't obtain device");
	
	vkGetDeviceQueue(ext.device, ext.queueFamilyIndex, 0, &ext.queue);

	Log::println("Successfully created device");
	delete[] families;

	#if defined(__DEBUG__) && defined(__WINDOWS__)

		//Debug object names

		ext.debugNames = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(ext.device, "vkSetDebugUtilsObjectNameEXT");

	#endif

	vkName(ext, ext.instance, VK_OBJECT_TYPE_INSTANCE, "Vulkan instance");
	vkName(ext, ext.device, VK_OBJECT_TYPE_DEVICE, "Vulkan device");

	//Create command pool

	VkCommandPoolCreateInfo poolInfo;
	memset(&poolInfo, 0, sizeof(poolInfo));

	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = ext.queueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	vkCheck<0xD>(vkCreateCommandPool(ext.device, &poolInfo, vkAllocator, &ext.pool), "Couldn't create command pool");
	vkName(ext, ext.pool, VK_OBJECT_TYPE_COMMAND_POOL, "Graphics command pool");

	//Get memory properties
	vkGetPhysicalDeviceMemoryProperties(ext.pdevice, &ext.pmemory);

	//Initialize resource commands
	ext.stagingCmdList = create("Resource command list", CommandListInfo());

}


void Graphics::initSurface(Window *w) {

	//Enable extension

	vkExtension(vkGetPhysicalDeviceSurfaceFormatsKHR);

	//Setup device surface (Uses our custom CMake defines to make this 'cross platform')
	__VK_SURFACE_TYPE__ surfaceInfo;
	memset(&surfaceInfo, 0, sizeof(surfaceInfo));
	surfaceInfo.sType = __VK_SURFACE_STYPE__;

	void *platformBegin = &surfaceInfo.__VK_SURFACE_HANDLE__;			//This is the start where platform dependent data starts
	memcpy(platformBegin, w->getSurfaceData(), w->getSurfaceSize());	//Memcpy the bytes from the window's representation of the surface

	vkCheck<0xE>(__VK_SURFACE_CREATE__(ext.instance, &surfaceInfo, vkAllocator, &ext.surface), "Couldn't obtain surface");

	//Check if the surface is supported

	VkBool32 supported = false;

	if (!vkCheck<0xF>(vkGetPhysicalDeviceSurfaceSupportKHR(ext.pdevice, 0, ext.surface, &supported), "Surface wasn't supported") || !supported)
		Log::throwError<VkGraphics, 0x1>("Surface wasn't supported");

	//Get the format we should display

	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ext.pdevice, ext.surface, &formatCount, NULL);

	if (formatCount == 0)
		Log::throwError<VkGraphics, 0x2>("Couldn't get surface format");

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

	if (size == Vec2u::max())
		Log::throwError<VkGraphics, 0x3>("Size is undefined; this is not supported!");

	if (size != w->getInfo().getSize())
		Log::throwError<VkGraphics, 0x4>(String("Render size didn't match ") + size);

	Log::println(String("Successfully created surface (") + size + ")");
	
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
		Log::throwError<VkGraphics, 0x5>("Immediate presentMode is required for single buffering");

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

	vkCheck<0x10>(vkCreateSwapchainKHR(ext.device, &swapchainInfo, vkAllocator, &ext.swapchain), "Couldn't create swapchain");
	vkName(ext, ext.swapchain, VK_OBJECT_TYPE_SWAPCHAIN_KHR, "Back buffer swapchain");

	vkGetSwapchainImagesKHR(ext.device, ext.swapchain, &buffering, nullptr);

	//Create the swapchain images
	
	std::vector<VkImage> swapchainImages = std::vector<VkImage>(buffering);
	vkGetSwapchainImagesKHR(ext.device, ext.swapchain, &buffering, swapchainImages.data());

	//Create present fence

	VkFenceCreateInfo fenceInfo;
	memset(&fenceInfo, 0, sizeof(fenceInfo));

	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	ext.presentFence.resize(buffering);
	for (u32 i = 0; i < buffering; ++i) {
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCheck<0x11>(vkCreateFence(ext.device, &fenceInfo, vkAllocator, ext.presentFence.data() + i), "Couldn't create the present fence");
		vkName(ext, ext.presentFence[i], VK_OBJECT_TYPE_FENCE, String("Present stall fence #") + i);
	}

	//Create semaphore

	VkSemaphoreCreateInfo semaphoreInfo;
	memset(&semaphoreInfo, 0, sizeof(semaphoreInfo));

	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ext.submitSemaphore.resize(buffering);
	ext.swapchainSemaphore.resize(buffering);

	for (u32 i = 0; i < buffering; ++i) {
		vkCheck<0x12>(vkCreateSemaphore(ext.device, &semaphoreInfo, vkAllocator, ext.submitSemaphore.data() + i), "Couldn't create submit semaphore");
		vkName(ext, ext.submitSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, String("Submit stall semaphore #") + i);
	}

	for (u32 i = 0; i < buffering; ++i) {
		vkCheck<0x13>(vkCreateSemaphore(ext.device, &semaphoreInfo, vkAllocator, ext.swapchainSemaphore.data() + i), "Couldn't create swapchain semaphore");
		vkName(ext, ext.swapchainSemaphore[i], VK_OBJECT_TYPE_SEMAPHORE, String("Swapchain stall semaphore #") + i);
	}

	Log::println(String("Successfully created swapchain (with buffering option ") + buffering + ") " + capabilities.minImageCount + " " + capabilities.maxImageCount);

	std::vector<Texture*> textures = std::vector<Texture*>(buffering);
	
	for (u32 i = 0; i < buffering; ++i) {
		
		Texture *tex = textures[i] = new Texture(TextureInfo(size, format, TextureUsage::Render_target));
		VkTexture &vkTex = tex->getExtension();
		vkTex.resource = swapchainImages[i];
		
		tex->g = this;
		tex->name = String("Swapchain image ") + i;
		tex->setHash<Texture>();

		if(!tex->init(false))
			Log::throwError<VkGraphics, 0x6>("Couldn't initialize swapchain image view");

		add(tex);
		use(tex);

	}

	VersionedTexture *vt = create("Swapchain images", VersionedTextureInfo(textures));
	use(vt);

	//Create depth buffer

	Texture *depthBuffer = create("Swapchain depth", TextureInfo(size, TextureFormat::Depth, TextureUsage::Render_depth));
	use(depthBuffer);

	Log::println("Successfully created image views of the swapchain");

	//Turn it into a RenderTarget aka 'Render pass'

	RenderTargetInfo info(size, depthBuffer->getFormat(), { VkTextureFormat(colorFormat).getName() });
	info.depth = depthBuffer;
	info.textures = { vt };

	backBuffer = new RenderTarget(info);

	backBuffer->g = this;
	backBuffer->setHash<RenderTarget>();
	backBuffer->name = "Swapchain";

	if(!backBuffer->init(false))
		Log::throwError<VkGraphics, 0x7>("Couldn't initialize back buffer (render target)");

	add(backBuffer);
	use(backBuffer);

	Log::println("Successfully created back buffer");

	ext.stagingBuffers.resize(buffering);
}

void Graphics::destroySurface() {

	if (ext.swapchain != VK_NULL_HANDLE) {

		finish();

		for (VkSemaphore &semaphore : ext.submitSemaphore)
			vkDestroySemaphore(ext.device, semaphore, vkAllocator);

		for (VkSemaphore &semaphore : ext.swapchainSemaphore)
			vkDestroySemaphore(ext.device, semaphore, vkAllocator);

		for (VkFence &fence : ext.presentFence)
			vkDestroyFence(ext.device, fence, vkAllocator);

		vkDestroySwapchainKHR(ext.device, ext.swapchain, vkAllocator);
		vkDestroySurfaceKHR(ext.instance, ext.surface, vkAllocator);

		destroy(backBuffer);
		ext.swapchain = VK_NULL_HANDLE;

		Log::println("Successfully destroyed surface");
	}

}

void Graphics::begin() {

	renderTimer.reset();

	u32 next = ext.frames == 0 ? 0 : (ext.current + 1) % buffering;

	//Get next image

	vkCheck<0x14>(vkAcquireNextImageKHR(ext.device, ext.swapchain, u64_MAX, ext.swapchainSemaphore[next], VK_NULL_HANDLE, &ext.current), "Couldn't acquire next image");

	renderTimer.lap("vkAcquireNextImageKHR");

	//Wait for previous frame

	vkCheck<0x15>(vkWaitForFences(ext.device, 1, ext.presentFence.data() + ext.current, VK_TRUE, u64_MAX), "Couldn't wait for fences");

	renderTimer.lap("vkWaitForFences");

	//Clean up staging buffers from previous frame

	std::vector<VkGPUBuffer> &stagingBuffers = ext.stagingBuffers[ext.current];

	if (stagingBuffers.size() != 0) {

		for (u32 i = 0, j = (u32) stagingBuffers.size(); i < j; ++i) {

			VkGPUBuffer &buffer = stagingBuffers[i];

			for(VkBuffer &vkBuffer : buffer.resource)
				vkDestroyBuffer(ext.device, vkBuffer, vkAllocator);

			vkFreeMemory(ext.device, buffer.memory, vkAllocator);
		}

		stagingBuffers.clear();

	}

	renderTimer.lap("Free staging buffers");

	//Reset fences

	vkCheck<0x16>(vkResetFences(ext.device, 1, ext.presentFence.data() + ext.current), "Couldn't reset fences");

	renderTimer.lap("vkResetFences");

}

void Graphics::end() {

	renderTimer.lap("Frame");

	//Submit commands

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));

	std::vector<GraphicsObject*> commandList = get<CommandList>();

	std::vector<VkCommandBuffer> commandBuffer;
	commandBuffer.reserve(commandList.size());

	//Submit staging commands; if possible

	std::vector<GraphicsObject*> buffers = get<GPUBuffer>();
	std::vector<GraphicsObject*> textures = get<Texture>();

	bool shouldStage = false;

	for (GraphicsObject *go : buffers)		//Check GPU buffers for updates (staging)
		if (((GPUBuffer*)go)->shouldStage()) {
			shouldStage = true;
			break;
		}

	if(!shouldStage)
		for (GraphicsObject *go : textures)		//Check textures for updates
			if (((Texture*)go)->shouldStage()) {
				shouldStage = true;
				break;
			}

	if (shouldStage)						//Start staging commands
		ext.stagingCmdList->begin();

	for (GraphicsObject *go : buffers)		//Push GPU buffers (some aren't staged)
		((GPUBuffer*)go)->push();

	if (shouldStage) {						//Push textures (all are staged)
		for (GraphicsObject *go : textures)
			((Texture*)go)->push();
	}

	if(shouldStage) {						//Put staging commands into command buffer
		ext.stagingCmdList->end();
		commandBuffer.push_back(ext.stagingCmdList->getExtension().cmd(ext));
	}

	//Submit user commands

	for (u32 i = 0; i < (u32)commandList.size(); ++i) {

		CommandList *cmdList = (CommandList*)commandList[i];

		if (cmdList != ext.stagingCmdList)
			commandBuffer.push_back(cmdList->ext.cmd(ext));
	}

	//Submit queue

	VkPipelineStageFlags stageWait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (u32)commandBuffer.size();
	submitInfo.pCommandBuffers = commandBuffer.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = ext.submitSemaphore.data() + ext.current;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = ext.swapchainSemaphore.data() + ext.current;
	submitInfo.pWaitDstStageMask = &stageWait;

	vkCheck<0x17>(vkQueueSubmit(ext.queue, 1, &submitInfo, ext.presentFence[ext.current]), "Couldn't submit queue");

	renderTimer.lap("vkQueueSubmit");

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
	presentInfo.pWaitSemaphores = ext.submitSemaphore.data() + ext.current;

	vkCheck<0x18>(vkQueuePresentKHR(ext.queue, &presentInfo), "Couldn't present image");
	vkCheck<0x19>(result, "Couldn't present image");

	renderTimer.lap("vkQueuePresentKHR");

	if (ext.frames % 100 == 0) {
		Log::println(String("Frame #") + ext.frames + ":");
		for (auto &elem : renderTimer.getTotalTime())
			Log::println(elem.first + "; " + elem.second);
	}

	++ext.frames;

}

void Graphics::finish() {
	vkQueueWaitIdle(ext.queue);
	ext.current = 0;
	ext.frames = 0;
}

void Window::updateAspect() {

	GraphicsInterface *irf = dynamic_cast<GraphicsInterface*>(wi);

	if (irf == nullptr)
		return;

	Graphics &g = irf->getGraphics();
	GraphicsExt &gext = g.getExtension();

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gext.pdevice, gext.surface, &capabilities);

	if (!initialized)
		info.flippedOnStart = capabilities.currentTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	
	info.flipped = capabilities.currentTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	if (wi != nullptr) {
		f32 aspect = Vec2(info.size).getAspect();
		wi->onAspectChange(info.flipped != info.flippedOnStart ? 1 / aspect : aspect);
	}

}

#endif