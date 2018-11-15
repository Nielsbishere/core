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

if(MSVC)

	# Executable and icon
	add_executable(
		app_pc
		${app_pc_SRC} res/app_pc.ico app_pc.rc
	)

	# Add a baker for the editor
	if(strip_debug_info)
		add_custom_command(TARGET app_pc COMMAND ${HOME_DIR}/oibaker.exe -strip_debug_info WORKING_DIRECTORY "${HOME_DIR}/app")
	else()
		add_custom_command(TARGET app_pc COMMAND ${HOME_DIR}/oibaker.exe WORKING_DIRECTORY "${HOME_DIR}/app")
	endif()
	
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
	set_target_properties(app_pc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app")
	set_target_properties(app_pc PROPERTIES OUTPUT_NAME "${APP_NAME}")
endif()