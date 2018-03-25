project(oic)

cmake_minimum_required(VERSION 3.9.0)

set(CMAKE_BUILD_TYPE Debug)

if(MSVC)
set(CMAKE_CXX_FLAGS "-std=c++14 /EHsc /W3")
else()
set(CMAKE_CXX_FLAGS "-std=c++17")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ostlc)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT owc)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(owc) 
