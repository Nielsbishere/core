include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(include)
include_directories(../deps/glslang)
include_directories(../deps)

file(GLOB_RECURSE vkogc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	vkogc STATIC
	${vkogc_SRC}
)

target_link_libraries(vkogc ostlc owc ogc)
target_link_libraries(vkogc glslang OGLCompiler OSDependent SPIRV SPVRemapper spirv-cross-core)

if(Vulkan)

	if(WIN32)
		if(DEFINED ENV{VULKAN_SDK})

			set(VULKAN_LIB vulkan-1)
			
			include_directories(vkogc PUBLIC $ENV{VULKAN_SDK}/Include)

			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				target_link_directories(vkogc PUBLIC $ENV{VULKAN_SDK}/Lib)
			else()
				target_link_directories(vkogc PUBLIC $ENV{VULKAN_SDK}/Lib32)
			endif()

		else()
			message(FATAL_ERROR "-- Vulkan - disabled: Please download the Vulkan SDK so you can compile")
		endif()
	else()
		set(VULKAN_LIB vulkan)
	endif()

	if(Windows)
		add_definitions(-D__VK_SURFACE_EXT__="VK_KHR_win32_surface")
		add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
	elseif(Android)
		add_definitions(-D__VK_SURFACE_EXT__="VK_KHR_android_surface")
		add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
	endif()
	
	target_link_libraries(vkogc ${VULKAN_LIB})
endif()