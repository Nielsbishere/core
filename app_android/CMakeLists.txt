include_directories(../ostlc/include)
include_directories(../owc/include)
include_directories(../ogc/include)
include_directories(../app/include)
include_directories(include)

file(GLOB_RECURSE app_android_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
	"${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
)

add_library(
	app_android SHARED
	${app_android_SRC}
)

add_definitions(-D__PROJECT_RES__="assets/")

target_link_libraries(app_android ostlc)
target_link_libraries(app_android owc)
target_link_libraries(app_android ogc)
target_link_libraries(app_android app)

include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
target_link_libraries(app_android android)

# Convert .so to .apk (Thanks to Discordia; check android_apk_make.txt)

set(ANDROID_NAME app_android)
set(ANDROID_DIR ${HOME_DIR}/${ANDROID_NAME})
set(APK_DIR ${CMAKE_SOURCE_DIR}/bin/apk)
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/lib)

set(ANDROID_APK_PACKAGE com.oi.${APP_PACKAGE})
set(ANDROID_APK_SIGNER_KEYSTORE	"~/my-release-key.keystore")
set(ANDROID_APK_SIGNER_ALIAS "myalias")

# Recreate the apk lib directory and copy the libs directory there

add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory "${APK_DIR}/libs/${ANDROID_ABI}")
add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${APK_DIR}/libs/${ANDROID_ABI}")
add_custom_command(TARGET ${ANDROID_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${LIBS_DIR}" "${APK_DIR}/libs/${ANDROID_ABI}")
  
# Set debuggable

if(CMAKE_BUILD_TYPE MATCHES Release)
   set(ANDROID_APK_DEBUGGABLE "false")
else()
   set(ANDROID_APK_DEBUGGABLE "true")
endif()

# Copy resources

add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${APK_DIR}/res")
add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${ANDROID_DIR}/res" "${APK_DIR}/res/")
add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${HOME_DIR}/app/res" "${APK_DIR}/assets/")

# Copy source files

add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${APK_DIR}/src")
add_custom_command(TARGET ${ANDROID_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${ANDROID_DIR}/src" "${APK_DIR}/src/")

# Copy manifest & strings.xml
configure_file("${ANDROID_DIR}/AndroidManifest.xml" "${APK_DIR}/AndroidManifest.xml")
configure_file("${ANDROID_DIR}/strings.xml" "${APK_DIR}/res/values/strings.xml")

# Refresh command
add_custom_command(TARGET ${ANDROID_NAME}
  COMMAND android update project -t android-${ANDROID_NATIVE_API_LEVEL} --name ${ANDROID_NAME} --path "${APK_DIR}")
  
# Build the apk file
if(CMAKE_BUILD_TYPE MATCHES Release)

  # Let Ant create the unsigned apk file
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND ant release WORKING_DIRECTORY "${APK_DIR}")

  # Sign the apk file
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND jarsigner -verbose -keystore ${ANDROID_APK_SIGNER_KEYSTORE} bin/${ANDROID_NAME}-unsigned.apk ${ANDROID_APK_SIGNER_ALIAS} WORKING_DIRECTORY "${APK_DIR}")

  # Align the apk file
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND zipalign -v -f 4 bin/${ANDROID_NAME}-unsigned.apk bin/${ANDROID_NAME}.apk WORKING_DIRECTORY "${APK_DIR}")
  
  # Install current version on the device/emulator
  if(ANDROID_APK_INSTALL OR ANDROID_APK_RUN)
  
    add_custom_command(TARGET ${ANDROID_NAME} COMMAND adb install -r bin/${ANDROID_NAME}.apk WORKING_DIRECTORY "${APK_DIR}")
	
  endif()
  
else()

  # Let Ant create the unsigned apk file
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND ant debug WORKING_DIRECTORY "${APK_DIR}")
  
  # Install current version on the device/emulator
  if(ANDROID_APK_INSTALL OR ANDROID_APK_RUN)
    add_custom_command(TARGET ${ANDROID_NAME} COMMAND adb install -r bin/${ANDROID_NAME}-debug.apk WORKING_DIRECTORY "${APK_DIR}")
  endif()
  
endif()

# Start the application
if(ANDROID_APK_RUN)
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND adb shell am start -n ${ANDROID_APK_PACKAGE}/android.app.NativeActivity)
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND adb logcat -c)
  add_custom_command(TARGET ${ANDROID_NAME} COMMAND adb logcat -s "oi_Log")
endif()