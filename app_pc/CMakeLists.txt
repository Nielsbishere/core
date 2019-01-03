include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE app_pc_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

if(WIN32)

	file(GLOB_RECURSE app_RES
		"../app/res/*"
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
	endif()
	
	file(WRITE "app_pc.rc" "")
	
	foreach(res ${app_RES})
		string(REPLACE "${CMAKE_SOURCE_DIR}/app_pc/" "" rcPath ${res})
		string(REPLACE "${CMAKE_SOURCE_DIR}/app_pc/../app/" "" resPath ${res})
		file(APPEND "app_pc.rc" "${resPath}               RCDATA               \"${rcPath}\"\n")
	endforeach()
	
	file(APPEND "app_pc.rc" "LOGO               ICON               \"res/app_pc.ico\"\n")

	file(GLOB_RECURSE app_pc_RES
		"res/*.ico"
		"app_pc.rc"
	)

	# Executable and icon
	add_executable(
		app_pc
		${app_pc_SRC}
		${app_RES}
		${app_pc_RES}
	)

else()

	add_executable(
		app_pc
		${app_pc_SRC}
	)
	
endif()

target_link_libraries(app_pc ostlc)
target_link_libraries(app_pc owc)
target_link_libraries(app_pc ogc)
target_link_libraries(app_pc app)

if(MSVC)
	set_target_properties(app_pc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app_pc")
	set_target_properties(app_pc PROPERTIES OUTPUT_NAME "${APP_NAME}")
endif()