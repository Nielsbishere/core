include_directories(../ostlc/include)
include_directories(include)

file(GLOB_RECURSE owc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	owc
	${owc_SRC}
)

target_link_libraries(owc ostlc)

if(WIN32)
target_link_libraries(owc Xinput.lib)
endif()