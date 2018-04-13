@echo off

set lvl=24
set abi=arm64-v8a
set dev=windows-x86_64

if "%1"=="" goto :error
set dev=%1
if "%2"=="" goto :error
set abi=%2
if "%3"=="" goto :error
set lvl=%3

:program
call clean
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-%lvl% -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK_HOME%\prebuilt\%dev%\bin\make.exe -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="%abi%" -DAndroid=ON -DANDROID_APK_RUN=ON .
mingw32-make -j
pause
goto :eof

:error
echo Insufficient params; Please use build_android currentPlatform target androidVersion
echo Example: build_android windows-x86_64 arm64-v8a 24
echo Running with params: %dev% %abi% %lvl%
goto :program