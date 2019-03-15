include_directories(../otc/include)
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
	"res/shaders/*.tese"
	"res/shaders/*.tesc"
	"res/shaders/*.rgen"
	"res/shaders/*.rint"
	"res/shaders/*.rahit"
	"res/shaders/*.rchit"
	"res/shaders/*.rmiss"
	"res/shaders/*.rcall"
	"res/shaders/*.glsl"
	"res/shaders/*.hlsl"
	"res/shaders/*.ogsl"
)

add_library(
	app STATIC
	${app_SRC}
)

target_link_libraries(app otc owc ogc)

if(Android)
	add_subdirectory(app_android)
	set_property(TARGET app_android PROPERTY FOLDER platform)
else()
	add_subdirectory(app_windows)
	set_property(TARGET app_windows PROPERTY FOLDER platform)
endif()