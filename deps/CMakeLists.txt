cmake_minimum_required(VERSION 3.9.0)
set(BUILD_TESTING OFF)

# zlib
add_subdirectory(custom/zlib)
set_property(TARGET zlib PROPERTY FOLDER zlib)

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
	target_compile_options(spirv-cross-glsl PRIVATE /W1)
	target_compile_options(spirv-cross-hlsl PRIVATE /W0)
	target_compile_options(spirv-cross-cpp PRIVATE /W0)
	target_compile_options(spirv-cross-msl PRIVATE /W0)
	target_compile_options(spirv-cross-reflect PRIVATE /W0)
	target_compile_options(spirv-cross-util PRIVATE /W0)
endif()