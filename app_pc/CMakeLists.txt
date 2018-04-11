include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
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
target_link_libraries(pc ogc)
target_link_libraries(pc app)

add_custom_command(TARGET pc PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${HOME_DIR}/app/res" "${CMAKE_SOURCE_DIR}/res/")