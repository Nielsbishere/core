include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)
include_directories(../deps/glslang)
include_directories(../deps)

file(GLOB_RECURSE ogc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
    "../deps/glslang/StandAlone/ResourceLimits.cpp"
)

add_library(
	ogc STATIC
	${ogc_SRC}
)

target_link_libraries(ogc ostlc owc)
target_link_libraries(ogc glslang OGLCompiler OSDependent SPIRV SPVRemapper spirv-cross-core)

# api dependent
if(Vulkan)
	add_subdirectory(vkogc)
	set_property(TARGET vkogc PROPERTY FOLDER api)
	target_link_libraries(ogc vkogc)
endif()