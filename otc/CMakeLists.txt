include_directories(include)
include_directories(../deps)
include_directories(../deps/rapidjson/include)

file(GLOB_RECURSE otc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

if(Android)

	file(GLOB_RECURSE platforms_SRC
		"platforms/android/include/*.h"
		"platforms/android/src/*.c"
		"platforms/android/include/*.hpp"
		"platforms/android/src/*.cpp"
	)

	add_library(
		otc STATIC
		${otc_SRC}
		${platforms_SRC}
	)

	target_link_libraries(otc android log)
	
else()

	file(GLOB_RECURSE platforms_SRC
		"platforms/windows/include/*.h"
		"platforms/windows/src/*.c"
		"platforms/windows/include/*.hpp"
		"platforms/windows/src/*.cpp"
	)

	add_library(
		otc STATIC
		${otc_SRC}
		${platforms_SRC}
	)
	
endif()

target_link_libraries(otc zlib)