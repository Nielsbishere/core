include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(include)

file(GLOB_RECURSE oish_gen_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	oish_gen
	${oish_gen_SRC}
)

target_link_libraries(oish_gen ostlc)
target_link_libraries(oish_gen owc)

if(Vulkan)
target_link_libraries(oish_gen ogc)
target_link_libraries(oish_gen ${VULKAN_LIB})
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()