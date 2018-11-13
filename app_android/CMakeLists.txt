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

# Convert .so to .apk (Thanks to Discordia; check android_apk_make.txt)

set(ANDROID_NAME app_android)
set(ANDROID_DIR ${HOME_DIR}/${ANDROID_NAME})
set(APK_DIR ../bin/build)
set(LIBS_DIR ../lib)

# Set debuggable

if(CMAKE_BUILD_TYPE MATCHES Release)
   set(ANDROID_APK_DEBUGGABLE "false")
   message("-- Debug - disabled")
else()
   set(ANDROID_APK_DEBUGGABLE "true")
   message("-- Debug - enabled")
endif()

# Configure android project
configure_file("${ANDROID_DIR}/AndroidManifest.xml" "${APK_DIR}/AndroidManifest.xml")
configure_file("${ANDROID_DIR}/strings.xml" "${APK_DIR}/res/values/strings.xml")
configure_file("${ANDROID_DIR}/build.xml" "${APK_DIR}/build.xml")