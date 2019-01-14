include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE app_android_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
	"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
)

add_library(
	app_android SHARED
	${app_android_SRC}
)

target_link_libraries(app_android ostlc)
target_link_libraries(app_android owc)
target_link_libraries(app_android ogc)
target_link_libraries(app_android app)

set(APK_DIR ../../bin/build)
set(ANDROID_NAME app_android)

# Set debuggable

if(CMAKE_BUILD_TYPE MATCHES Release)
   set(ANDROID_APK_DEBUGGABLE "false")
   message("-- Debug - disabled")
else()
   set(ANDROID_APK_DEBUGGABLE "true")
   message("-- Debug - enabled")
endif()

# Configure android project
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/build/AndroidManifest.xml")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/strings.xml" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/build/res/values/strings.xml")