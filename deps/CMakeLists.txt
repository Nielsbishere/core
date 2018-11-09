cmake_minimum_required(VERSION 3.9.0)
set(BUILD_TESTING OFF)

# zlib
add_subdirectory(custom/zlib)
set_property(TARGET zlib PROPERTY FOLDER zlib)

# spirv-tools
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "Skip building the executable and tests along with the library" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "Skip installation" FORCE)
set(SPIRV-Headers_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Headers")
add_subdirectory(SPIRV-Tools)

set_property(TARGET spirv-tools-pkg-config spirv-tools-shared-pkg-config PROPERTY FOLDER SPIRV-Tools\ build)

# glslang
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
add_subdirectory(glslang)
set_property(TARGET HLSL PROPERTY FOLDER glslang)

# spirv-cross
add_subdirectory(SPIRV-Cross)
set_property(TARGET spirv-cross spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util spirv-cross-cpp PROPERTY FOLDER spirv-cross)

# suppress warnings that aren't mine
if(MSVC)

	target_compile_options(zlib PRIVATE /W0)
	
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