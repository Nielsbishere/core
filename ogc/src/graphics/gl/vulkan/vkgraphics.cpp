#ifdef __VULKAN__

#include <cstring>
#include "window/window.h"
#include "graphics/graphics.h"
#include "graphics/interface/graphicsinterface.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/render/commandlist.h"


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

	u32 layerCount, extensionCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

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
	
	for(u32 i = 0; i < extensionCount; ++i)
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

	Log::println("\tExtensions:");

	for (auto exten : cextensions)
		Log::println(String("\t\t") + exten);

	Log::println("\tLayers:");

	for (auto lay : clayers)
		Log::println(String("\t\t") + lay);

	vkCheck<0x1>(vkCreateInstance(&instanceInfo, vkAllocator, &ext.instance), "Couldn't obtain Vulkan instance");
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

	vkCheck<0x19>(vkCreateDebugReportCallbackEXT(ext.instance, &callbackInfo, vkAllocator, &ext.debugCallback), "Couldn't create debug report callback");

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

	vkGetPhysicalDeviceFeatures(ext.pdevice, &ext.pfeatures);

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
	
	vkCheck<0x2>(vkCreateDevice(*gpu, &deviceInfo, vkAllocator, &ext.device), "Couldn't obtain device");
	
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

	vkCheck<0x16>(vkCreateCommandPool(ext.device, &poolInfo, vkAllocator, &ext.pool), "Couldn't create command pool");
	vkName(ext, ext.pool, VK_OBJECT_TYPE_COMMAND_POOL, "Graphics command pool");

	//Get memory properties
	vkGetPhysicalDeviceMemoryProperties(ext.pdevice, &ext.pmemory);

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

	vkCheck<0x3>(__VK_SURFACE_CREATE__(ext.instance, &surfaceInfo, vkAllocator, &ext.surface), "Couldn't obtain surface");

	//Check if the surface is supported

	VkBool32 supported = false;

	if (!vkCheck<0x4>(vkGetPhysicalDeviceSurfaceSupportKHR(ext.pdevice, 0, ext.surface, &supported), "Surface wasn't supported") || !supported)
		Log::throwError<Graphics, 0x5>("Surface wasn't supported");

	//Get the format we should display

	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(ext.pdevice, ext.surface, &formatCount, NULL);

	if (formatCount == 0)
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

	if (size == Vec2u::max())
		Log::throwError<Graphics, 0x7>("Size is undefined; this is not supported!");

	if (size != w->getInfo().getSize())
		Log::throwError<Graphics, 0x8>(String("Render size didn't match ") + size);

	Log::println(String("Successfully created surface (") + size + ")");
	
	//Create swapchain
	
	VkSwapchainCreateInfoKHR swapchainInfo;
	memset(&swapchainInfo, 0, sizeof(swapchainInfo));
	
	//Try to support mailbox present mode if triple buffering is enabled
	
	Log::println(String("Attempting ") + buffering);

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

	Log::println(String("Option: ") + buffering);

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

	vkCheck<0x9>(vkCreateSwapchainKHR(ext.device, &swapchainInfo, vkAllocator, &ext.swapchain), "Couldn't create swapchain");
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

	ext.fences.resize(2 * buffering);

	for (u32 i = 0; i < 2 * buffering; ++i) {
		fenceInfo.flags = i < buffering ? 0 : VK_FENCE_CREATE_SIGNALED_BIT;
		vkCheck<0xE>(vkCreateFence(ext.device, &fenceInfo, vkAllocator, ext.fences.data() + i), "Couldn't create the present fence");
		vkName(ext, ext.fences[i], VK_OBJECT_TYPE_FENCE, String("Present fence # ") + i / buffering + " version " + i % buffering);
	}

	//Create semaphore

	VkSemaphoreCreateInfo semaphoreInfo;
	memset(&semaphoreInfo, 0, sizeof(semaphoreInfo));

	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	ext.semaphores.resize(buffering);

	for (u32 i = 0; i < buffering; ++i) {
		vkCheck<0x17>(vkCreateSemaphore(ext.device, &semaphoreInfo, vkAllocator, ext.semaphores.data() + i), "Couldn't create semaphore");
		vkName(ext, ext.semaphores[i], VK_OBJECT_TYPE_SEMAPHORE, String("Present semaphore version ") + i);
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
			Log::throwError<Graphics, 0xA>("Couldn't initialize swapchain image view");

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
		Log::throwError<Graphics, 0xC>("Couldn't initialize back buffer (render target)");

	add(backBuffer);
	use(backBuffer);

	Log::println("Successfully created back buffer");
}

void Graphics::destroySurface() {

	if (ext.swapchain != VK_NULL_HANDLE) {

		finish();

		for (VkSemaphore &semaphore : ext.semaphores)
			vkDestroySemaphore(ext.device, semaphore, vkAllocator);

		for (VkFence &fence : ext.fences)
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
	Log::println(String("Locking fence: ") + next);

	vkCheck<0x10>(vkAcquireNextImageKHR(ext.device, ext.swapchain, u64_MAX, VK_NULL_HANDLE, ext.fences[next], &ext.current), "Couldn't acquire next image");

	renderTimer.lap("vkAcquireNextImageKHR");

	//Wait for image and previous frame

	VkFence fences[2] = { ext.fences[ext.current], ext.fences[ext.current + buffering] };

	Log::println(String("Unlocking fences: ") + ext.current + " and " + (ext.current + buffering) + " (" + next + ")");

	vkCheck<0x11>(vkWaitForFences(ext.device, 2, fences, VK_TRUE, u64_MAX), "Couldn't wait for fences");

	renderTimer.lap("vkWaitForFences");

	//Reset fences

	vkCheck<0x12>(vkResetFences(ext.device, 2, fences), "Couldn't reset fences");

	renderTimer.lap("vkResetFences");

}

void Graphics::end() {

	renderTimer.lap("Frame");

	//Submit commands

	VkSubmitInfo submitInfo;
	memset(&submitInfo, 0, sizeof(submitInfo));

	std::vector<GraphicsObject*> commandList = get<CommandList>();

	std::vector<VkCommandBuffer> commandBuffer(commandList.size());
	for (u32 i = 0; i < (u32) commandBuffer.size(); ++i)
		commandBuffer[i] = ((CommandList*)commandList[i])->ext.cmd(ext);

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (u32) commandList.size();
	submitInfo.pCommandBuffers = commandBuffer.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = ext.semaphores.data() + ext.current;

	Log::println(String("Locking fence: ") + (ext.current + buffering));

	vkCheck<0x18>(vkQueueSubmit(ext.queue, 1, &submitInfo, ext.fences[ext.current + buffering]), "Couldn't submit queue");

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
	presentInfo.pWaitSemaphores = ext.semaphores.data() + ext.current;

	vkCheck<0xF>(vkQueuePresentKHR(ext.queue, &presentInfo), "Couldn't present image");
	vkCheck<0x14>(result, "Couldn't present image");

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

CommandList *Graphics::create(String name, CommandListInfo info) {

	CommandList *cl = new CommandList(info);

	cl->getExtension().pool = ext.pool;
	cl->g = this;
	cl->setHash<CommandList>();
	cl->name = name;

	if (!cl->init())
		Log::throwError<Graphics, 0x15>("Couldn't create command list");

	add(cl);
	return cl;
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