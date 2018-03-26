include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)

file(GLOB_RECURSE test_android_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	test_android SHARED
	${test_android_SRC}
)

target_link_libraries(test_android ostlc)
target_link_libraries(test_android owc)

if(Android)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
target_link_libraries(test_android android)
endif()