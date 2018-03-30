include_directories(../ostlc/include)
include_directories(include)

if(Android)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

file(GLOB_RECURSE owc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	owc STATIC
	${owc_SRC}
)

target_link_libraries(owc ostlc)

if(WIN32)
target_link_libraries(owc Xinput.lib)
elseif(Android)
target_link_libraries(owc android egl)
endif()