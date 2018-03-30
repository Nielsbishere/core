#include <template/platformdefines.h>

#ifdef __VULKAN__

#include <graphics/graphics.h>
#include <vulkan/vulkan.h>
#include <window/window.h>

#include <cstring>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;


struct VKGraphics {
	VkInstance instance;
	VkPhysicalDevice pdevice;
	VkDevice device;
	VkSurfaceKHR surface;
};

template<u32 errorId>
bool throwVkResult(VkResult result, String msg){
	
	if(result >= VK_SUCCESS) return false;
	
	switch(result){
		
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			Log::error("VkResult: Out of host memory");
			break;
		
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			Log::error("VkResult: Out of device memory");
			break;
		
		case VK_ERROR_INITIALIZATION_FAILED:
			Log::error("VkResult: Initialization failed");
			break;
		
		case VK_ERROR_DEVICE_LOST:
			Log::error("VkResult: Device lost");
			break;
		
		case VK_ERROR_MEMORY_MAP_FAILED:
			Log::error("VkResult: Memory map failed");
			break;
		
		case VK_ERROR_LAYER_NOT_PRESENT:
			Log::error("VkResult: Layer not present");
			break;
		
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			Log::error("VkResult: Extension not present");
			break;
		
		case VK_ERROR_FEATURE_NOT_PRESENT:
			Log::error("VkResult: Feature not present");
			break;
		
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			Log::error("VkResult: Incompatible driver");
			break;
		
		case VK_ERROR_TOO_MANY_OBJECTS:
			Log::error("VkResult: Too many objects");
			break;
		
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			Log::error("VkResult: Format not supported");
			break;
			
		default:
			Log::error("VkResult: Fragmented pool");
			break;
		
	}
	
	Log::throwError<Graphics, errorId>(msg);
	return true;
}

#define vkExtension(x) PFN_##x x = (PFN_##x) vkGetInstanceProcAddr(graphics.instance, #x); if (x == nullptr) oi::Log::throwError<VKGraphics, 0x0>("Couldn't get Vulkan extension");

//TODO: Allocator
#define allocator nullptr

Graphics::~Graphics(){
	
	if(initialized){
		VKGraphics &graphics = *(VKGraphics*) platformData;
		vkDestroyInstance(graphics.instance, allocator);
	}
	
}

void Graphics::init(Window *w){
	
	if(sizeof(VKGraphics) > Graphics::platformSize)
		Log::throwError<Graphics, 0x0>("Graphics struct can't contain VKGraphics");
	
	VKGraphics &graphics = *(VKGraphics*) platformData;
	std::memset(&graphics, 0, sizeof(VKGraphics));

	//Constants
	
	const u32 majorVersion = 1, minorVersion = 0, patchVersion = 0;					///Vulkan version
	
	std::vector<const char*> clayers, cextensions = std::vector<const char*>(2);	///Instance layers and extensions
	cextensions[0] = "VK_KHR_surface";
	
	#ifdef __ANDROID__
	cextensions[1] = "VK_KHR_android_surface";
	#elif __WINDOWS__
	cextensions[1] = "VK_KHR_windows_surface";
	#endif
	
	std::vector<const char*> dlayers, dextensions;									///Device layers and extensions
	
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
	
	throwVkResult<0x1>(vkCreateInstance(&instanceInfo, allocator, &graphics.instance), "Couldn't obtain Vulkan instance");
	initialized = true;
	
	Log::println("Successfully initialized Graphics with Vulkan context");
	
	//Show all layers and extensions
	
	#ifdef __DEBUG__
	
	uint32_t layerCount, extensionCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	
	VkLayerProperties *layers = new VkLayerProperties[layerCount];
	vkEnumerateInstanceLayerProperties(&layerCount, layers);
	
	VkExtensionProperties *extensions = new VkExtensionProperties[extensionCount];
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
	
	Log::println(String("Vulkan layers: ") + layerCount);
	for(uint32_t i = 0; i < layerCount; ++i){
		VkLayerProperties &l = layers[i];
		Log::println(String("Layer #") + i + ": " + l.layerName + " v" + l.specVersion + "-" + l.implementationVersion + ": " + l.description);
	}
	
	Log::println(String("Vulkan extensions: ") + extensionCount);
	for(uint32_t i = 0; i < extensionCount; ++i){
		VkExtensionProperties &e = extensions[i];
		Log::println(String("Extension #") + i + ": " + e.extensionName + " v" + e.specVersion);
	}
	
	delete[] layers;
	delete[] extensions;
	
	#endif
	
	//Get all devices
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, nullptr);
	
	VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(graphics.instance, &deviceCount, devices);
	
	#ifdef __DEBUG__
	Log::println(String("Vulkan devices: ") + deviceCount);
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
	
	float queuePriorities[] = { 1.f };
	uint32_t queueCount = (uint32_t)(sizeof(queuePriorities) / sizeof(float));
	VkDeviceQueueCreateInfo queues[queueCount];
	std::memset(queues, 0, sizeof(queues));
	
	queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues[0].queueCount = queueCount;
	queues[0].pQueuePriorities = queuePriorities;
	
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queues;
	
	throwVkResult<0x2>(vkCreateDevice(*gpu, &deviceInfo, allocator, &graphics.device), "Couldn't obtain Vulkan device");
	
	Log::println("Successfully created Vulkan device");
	
	//Setup surface
	
	vkExtension(vkGetPhysicalDeviceSurfaceFormatsKHR);
	//Set up device surface
	
}

void Graphics::clear(Vec4f color) {
	//TODO: Clear screen
}

#endif