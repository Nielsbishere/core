include_directories(include)
include_directories(../deps)
include_directories(../deps/rapidjson/include)

file(GLOB_RECURSE ostlc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	ostlc STATIC
	${ostlc_SRC}
)

target_link_libraries(ostlc zlib)

if(Android)
	target_link_libraries(ostlc android log)
endif()