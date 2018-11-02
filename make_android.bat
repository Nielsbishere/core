@echo off
echo Android requires the following dependencies to be installed:
echo - Java
echo - Android SDK
echo - Android NDK
echo - CMake
echo - MinGW Makefiles (64-bit)
echo - Apache Ant
echo - Vulkan SDK
setlocal enabledelayedexpansion

set lvl=24
set abi=arm64-v8a
set dev=windows-x86_64

if "%1"=="" goto :error
set dev=%1
if "%2"=="" goto :error
set lvl=%2
if "%3"=="" goto :error
set abi=%3

call :program
endlocal
pause
goto :eof

:program
echo Making Android build (%dev% %lvl% !abi!)
mkdir "builds" 2>nul
mkdir "builds/Android" 2>nul
mkdir "builds/Android/!abi!" 2>nul
cd "builds/Android/!abi!"
cmake "../../../" -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-%lvl% -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK_HOME%\prebuilt\%dev%\bin\make.exe -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="!abi!" -DAndroid=ON -DANDROID_APK_RUN=ON -DANDROID_STL=c++_shared
echo mingw32-make -j > run_android.bat
echo pause >> run_android.bat
cd "%~dp0"
exit /b

:error

echo Creating builds for every Android ABI...
echo For a specialized build: run_android windows-x86_64 24 arm64-v8a

call :program

set abi=armeabi-v7a
call :program

set abi=x86
call :program

set abi=x86_64
call :program

endlocal
goto :eof