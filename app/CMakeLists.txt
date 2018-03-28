include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)

file(GLOB_RECURSE app_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	app STATIC
	${app_SRC}
)

target_link_libraries(app ostlc)
target_link_libraries(app owc)