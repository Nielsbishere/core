file(GLOB_RECURSE ostlc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

include_directories(include)

add_library(
	ostlc STATIC
	${ostlc_SRC}
)

if(Android)
	target_link_libraries(ostlc android log)
endif()