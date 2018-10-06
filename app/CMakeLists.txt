include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(include)

file(GLOB_RECURSE app_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
	"res/shaders/*.frag"
	"res/shaders/*.geom"
	"res/shaders/*.vert"
	"res/shaders/*.comp"
	"res/shaders/*.glsl"
	"res/shaders/*.hlsl"
)

add_library(
	app STATIC
	${app_SRC}
)

target_link_libraries(app ostlc)
target_link_libraries(app owc)
target_link_libraries(app ogc)

add_custom_command(TARGET app COMMAND "${HOME_DIR}/prepare_resources")

if(MSVC)
	set_target_properties(app PROPERTIES VS_GLOBAL_DisableFastUpToDateCheck true)
endif()