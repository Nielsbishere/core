include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(include)

file(GLOB_RECURSE oibaker_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	oibaker
	${oibaker_SRC}
)

target_link_libraries(oibaker ostlc owc ogc)