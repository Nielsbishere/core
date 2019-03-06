include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE app_windows_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

if(Windows)

	file(GLOB_RECURSE app_RES
		"../res/*"
	)
	
	if(exclude_ext_formats)
		list(FILTER app_RES EXCLUDE REGEX ".*oiBM$")
		list(FILTER app_RES EXCLUDE REGEX ".*fbx$")
		list(FILTER app_RES EXCLUDE REGEX ".*obj$")
		list(FILTER app_RES EXCLUDE REGEX ".*glsl$")
		list(FILTER app_RES EXCLUDE REGEX ".*hlsl$")
		list(FILTER app_RES EXCLUDE REGEX ".*vert$")
		list(FILTER app_RES EXCLUDE REGEX ".*frag$")
		list(FILTER app_RES EXCLUDE REGEX ".*geom$")
		list(FILTER app_RES EXCLUDE REGEX ".*comp$")
		list(FILTER app_RES EXCLUDE REGEX ".*tese$")
		list(FILTER app_RES EXCLUDE REGEX ".*tesc$")
		list(FILTER app_RES EXCLUDE REGEX ".*rgen$")
		list(FILTER app_RES EXCLUDE REGEX ".*rmiss$")
		list(FILTER app_RES EXCLUDE REGEX ".*rahit$")
		list(FILTER app_RES EXCLUDE REGEX ".*rchit$")
		list(FILTER app_RES EXCLUDE REGEX ".*rint$")
		list(FILTER app_RES EXCLUDE REGEX ".*rcall$")
		list(FILTER app_RES EXCLUDE REGEX ".*ogsl$")
	endif()
	
	file(WRITE "app_windows.rc" "")
	
	foreach(res ${app_RES})
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/../" "../../" rcPath ${res})
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/../" "" resPath ${res})
		file(APPEND "app_windows.rc" "${resPath}               RCDATA               \"${rcPath}\"\n")
	endforeach()
	
	file(APPEND "app_windows.rc" "LOGO               ICON               \"res/app_windows.ico\"\n")

	file(GLOB_RECURSE app_windows_RES
		"res/app_windows.ico"
		"app_windows.rc"
	)

	# Executable and icon
	add_executable(
		app_windows
		${app_windows_SRC}
		${app_RES}
		${app_windows_RES}
	)
	
endif()

target_link_libraries(app_windows ostlc owc ogc app)

if(MSVC)
	set_target_properties(app_windows PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	set_target_properties(app_windows PROPERTIES OUTPUT_NAME "${APP_NAME}")
endif()