cmake_minimum_required(VERSION 3.9.0)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_CURRENT_SOURCE_DIR "${HOME_DIR}/deps/zlib")

project(zlib C)

set(VERSION "1.2.11")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Installation directory for manual pages")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

# configure zlib.pc and zconf.h

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc.cmakein
		${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc @ONLY)

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
		${CMAKE_BINARY_DIR}/zconf.h @ONLY)

# zlib

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_BINARY_DIR}/zconf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/crc32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/deflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gzguts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inffast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inffixed.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inftrees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zutil.h
)
set(ZLIB_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gzwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/zutil.c
)

add_library(zlib STATIC ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS} ${ZLIB_SRCS})
