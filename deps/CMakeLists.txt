cmake_minimum_required(VERSION 3.6.0)
set(BUILD_TESTING OFF)

# zlib
add_subdirectory(custom/zlib)
set_property(TARGET zlib PROPERTY FOLDER zlib)

# suppress warnings
if(MSVC)
	target_compile_options(zlib PRIVATE /W0)
endif()