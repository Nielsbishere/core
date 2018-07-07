include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(include)

file(GLOB_RECURSE ogc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	ogc STATIC
	${ogc_SRC}
)

target_link_libraries(ogc ostlc)
target_link_libraries(ogc owc)

if(Vulkan)
target_link_libraries(ogc ${VULKAN_LIB})
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()