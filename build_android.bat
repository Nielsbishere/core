@echo off

if "%1"=="" goto :error
if "%2"=="" goto :error
if "%3"=="" goto :error

cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-%3 -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK_HOME%\prebuilt\%2\bin\make.exe -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="%1" -DAndroid=ON .
pause
goto :eof

:error
echo Invalid usage; Please use build_android target currentPlatform androidVersion
echo Example: build_android arm64-v8a windows-x86_64 24
pause
goto :eof