include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE pc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	pc
	${pc_SRC}
)

target_link_libraries(pc ostlc)
target_link_libraries(pc owc)
target_link_libraries(pc app)