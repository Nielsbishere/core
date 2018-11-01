project(oic)

cmake_minimum_required(VERSION 3.9.0)

# User options

set(APP_NAME Osomi\ Core)
set(APP_PACKAGE Osomi_Core)
set(IS_GAME true)
set(APP_VERSION 1.0) 

# Default options

add_definitions(-D__PROJECT_NAME__="${APP_NAME}")

set(HOME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable multi processor compilation. Use C++ standard and enable all warnings (except nameless structs)
if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /MP /W4 /permissive- /wd4201")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /MP /W4 /permissive- /wd4201")
endif()

# No console/log option

option(No_log "No_log" OFF)

if(No_log)
add_definitions(-DNO_LOG)
endif()

# Android

option(Android "Android" OFF)

if(Android)

if(DEFINED ENV{JAVA_HOME} AND DEFINED ENV{ANDROID_HOME} AND DEFINED ENV{ANDROID_NDK_HOME})

	message("-- Android - enabled")

	set(ANDROID_NDK, $ENV{ANDROID_NDK_HOME})
	set(ANDROID_HOME, $ENV{ANDROID_HOME})
	set(JAVA_HOME, $ENV{JAVA_HOME})

	if(${ANDROID_ABI} STREQUAL "arm64-v8a")
		include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
	else()
		include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
	endif()
	
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

	set(CMAKE_VERBOSE_MAKEFILE 0)

	set(Vulkan ON)
	set(Desktop OFF)
	add_definitions(-D__PHONE__)
	
else()

	message(FATAL_ERROR "-- Android - disabled: please set JAVA_HOME, ANDROID_HOME and ANDROID_NDK_HOME in system variables!")

endif()

elseif(WIN32)

	set(Vulkan ON)
	add_definitions(-D__DESKTOP__)
	add_definitions(-D__WINDOWS__)
	set(Windows ON)
	set(Desktop ON)

else()

endif()

if(Vulkan)

	add_definitions(-D__VULKAN__)
	message("-- Vulkan - enabled")

	if(WIN32)
		if(DEFINED ENV{VULKAN_SDK})

			set(VULKAN_LIB vulkan-1)
			
			include_directories($ENV{VULKAN_SDK}/Include)

			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				link_directories($ENV{VULKAN_SDK}/Lib)
			else()
				link_directories($ENV{VULKAN_SDK}/Lib32)
			endif()

		else()
			message(FATAL_ERROR "-- Vulkan - disabled: Please download the Vulkan SDK so you can compile")
		endif()
	else()
		set(VULKAN_LIB vulkan)
	endif()

	if(Windows)
		add_definitions(-D__VK_SURFACE_TYPE__=VkWin32SurfaceCreateInfoKHR)
		add_definitions(-D__VK_SURFACE_STYPE__=VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR)
		add_definitions(-D__VK_SURFACE_CREATE__=vkCreateWin32SurfaceKHR)
		add_definitions(-D__VK_SURFACE_HANDLE__=hinstance)
		add_definitions(-D__VK_SURFACE_EXT__="VK_KHR_win32_surface")
		add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
	elseif(Android)
		add_definitions(-D__VK_SURFACE_TYPE__=VkAndroidSurfaceCreateInfoKHR)
		add_definitions(-D__VK_SURFACE_STYPE__=VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR)
		add_definitions(-D__VK_SURFACE_CREATE__=vkCreateAndroidSurfaceKHR)
		add_definitions(-D__VK_SURFACE_HANDLE__=window)
		add_definitions(-D__VK_SURFACE_EXT__="VK_KHR_android_surface")
		add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
	endif()
	
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-D__DEBUG__)
endif()

# dependencies
add_subdirectory(deps)

# ocore
add_subdirectory(ostlc)
add_subdirectory(owc)
add_subdirectory(ogc)
set_property(TARGET ostlc owc ogc PROPERTY FOLDER ocore)

# desktop conversion tools
add_subdirectory(oirm_gen)
add_subdirectory(oish_gen)

set_property(TARGET oirm_gen oish_gen PROPERTY FOLDER tools)

# app projects
add_subdirectory(app)
set_property(TARGET app PROPERTY FOLDER application)
	
if(Android)
	add_subdirectory(app_android)
	set_property(TARGET app_android PROPERTY FOLDER application)
else()
	add_subdirectory(app_pc)
	set_property(TARGET app_pc PROPERTY FOLDER application)
endif()
