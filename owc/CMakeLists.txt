include_directories(../otc/include)
include_directories(include)
include_directories(platforms/include)

if(Android)
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

file(GLOB_RECURSE owc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

if(Android)

	file(GLOB_RECURSE platforms_SRC
		"platforms/include/android/*.h"
		"platforms/src/android/*.c"
		"platforms/include/android/*.hpp"
		"platforms/src/android/*.cpp"
	)

	add_library(
		owc STATIC
		${owc_SRC}
		${platforms_SRC}
	)
	
else()

	file(GLOB_RECURSE platforms_SRC
		"platforms/include/windows/*.h"
		"platforms/src/windows/*.c"
		"platforms/include/windows/*.hpp"
		"platforms/src/windows/*.cpp"
	)

	add_library(
		owc STATIC
		${owc_SRC}
		${platforms_SRC}
	)
	
endif()

target_link_libraries(owc otc)