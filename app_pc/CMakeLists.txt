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

set_target_properties(app_pc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app")

set_target_properties(app_pc PROPERTIES OUTPUT_NAME "${APP_NAME}")

if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()