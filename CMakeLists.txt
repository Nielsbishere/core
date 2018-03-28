project(oic)

cmake_minimum_required(VERSION 3.9.0)

set(HOME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Default options

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Android

option(Android "Android" OFF)

if(Android)

message("-- Android - enabling")

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

set(CMAKE_VERBOSE_MAKEFILE 0)

add_subdirectory(app_android)

else()

message(ERROR "-- Android - disabled: please set JAVA_HOME, ANDROID_HOME and ANDROID_NDK_HOME in system variables!")

endif()

endif()

# Projects

add_subdirectory(ostlc)
add_subdirectory(owc)
add_subdirectory(app)

add_subdirectory(app_pc)