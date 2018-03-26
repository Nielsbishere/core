include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)

file(GLOB_RECURSE test_pc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	test_pc
	${test_pc_SRC}
)

target_link_libraries(test_pc ostlc)
target_link_libraries(test_pc owc)
