include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../owc/platforms/include)
include_directories(deps)
include_directories(include)

file(GLOB_RECURSE ogc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

if(shader_compilation)

	include_directories(deps/glslang)

	add_library(
		ogc STATIC
		${ogc_SRC}
		"deps/glslang/StandAlone/ResourceLimits.cpp"
	)

else()
	add_library(
		ogc STATIC
		${ogc_SRC}
	)
endif()

target_link_libraries(ogc ostlc owc)

# api dependent
if(Vulkan)
	add_subdirectory(vkogc)
	set_property(TARGET vkogc PROPERTY FOLDER api)
	target_link_libraries(ogc vkogc)
endif()

# if we want shader compilation, we need these libraries
if(shader_compilation)

	set(BUILD_TESTING OFF)
	
	# spirv-tools
	set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "Skip building the executable and tests along with the library" FORCE)
	set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "Skip installation" FORCE)
	set(SPIRV-Headers_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/SPIRV-Headers")
	set(ENABLE_TESTS OFF CACHE BOOL "Enable tests" FORCE)
	add_subdirectory(deps/SPIRV-Tools)
	
	set_property(TARGET spirv-tools-pkg-config spirv-tools-shared-pkg-config PROPERTY FOLDER SPIRV-Tools\ build)
	
	# glslang
	set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
	add_subdirectory(deps/glslang)
	set_property(TARGET HLSL PROPERTY FOLDER glslang)
	
	# spirv-cross
	add_subdirectory(deps/SPIRV-Cross)
	set_property(TARGET spirv-cross spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util spirv-cross-cpp spirv-cross-c spirv-cross-c-api-test extinst_tables enum_string_mapping core_tables PROPERTY FOLDER spirv-cross)

	# suppress warnings that aren't mine
	if(MSVC)
	
		target_compile_options(glslang PRIVATE /W0)
		target_compile_options(HLSL PRIVATE /W0)
		target_compile_options(OGLCompiler PRIVATE /W0)
		target_compile_options(OSDependent PRIVATE /W0)
		target_compile_options(SPIRV PRIVATE /W0)
		target_compile_options(glslang PRIVATE /W0)
		
		target_compile_options(spirv-cross PRIVATE /W0)
		target_compile_options(spirv-cross-core PRIVATE /W0)
		target_compile_options(spirv-cross-glsl PRIVATE /W0)
		target_compile_options(spirv-cross-hlsl PRIVATE /W0)
		target_compile_options(spirv-cross-cpp PRIVATE /W0)
		target_compile_options(spirv-cross-msl PRIVATE /W0)
		target_compile_options(spirv-cross-reflect PRIVATE /W0)
		target_compile_options(spirv-cross-util PRIVATE /W0)
		
		target_compile_options(SPIRV-Tools PRIVATE /W0)
		target_compile_options(SPIRV-Tools-link PRIVATE /W0)
		target_compile_options(SPIRV-Tools-opt PRIVATE /W0)
		target_compile_options(SPIRV-Tools-shared PRIVATE /W0)
	
	endif()

	target_link_libraries(ogc glslang OGLCompiler OSDependent SPIRV SPVRemapper spirv-cross-core)

endif()