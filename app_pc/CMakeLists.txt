include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE app_pc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	app_pc
	${app_pc_SRC}
)

target_link_libraries(app_pc ostlc)
target_link_libraries(app_pc owc)
target_link_libraries(app_pc ogc)
target_link_libraries(app_pc app)

add_custom_command(TARGET app_pc PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${HOME_DIR}/app/res" "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/res/")
add_custom_command(TARGET app_pc PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${HOME_DIR}/app/res" "${CMAKE_CURRENT_SOURCE_DIR}/res/")

set_target_properties(app_pc PROPERTIES OUTPUT_NAME "${APP_NAME}")
