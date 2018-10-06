include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)
include_directories(../deps/glslang)

file(GLOB_RECURSE ogc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
	"../deps/SPIRV-Cross/spirv_cross.cpp"
	"../deps/SPIRV-Cross/spirv_cfg.cpp"
	"../deps/SPIRV-Cross/spirv_glsl.cpp"
	"../deps/SPIRV-Cross/spirv_hlsl.cpp"
	"../deps/glslang/StandAlone/ResourceLimits.cpp"
)

add_library(
	ogc STATIC
	${ogc_SRC}
)

target_link_libraries(ogc ostlc)
target_link_libraries(ogc owc)
target_link_libraries(ogc glslang OGLCompiler OSDependent SPIRV SPVRemapper)

if(Vulkan)
	target_link_libraries(ogc ${VULKAN_LIB})
endif()