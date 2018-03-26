@echo off

if "%1"=="" goto :error
if "%2"=="" goto :error

cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-21 -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK_HOME%\prebuilt\%2\bin\make.exe -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="%1" -DAndroid=ON .
mingw32-make
pause
goto :eof

:error
echo Invalid usage; Please use build_android target currentPlatform
echo Example: build_android arm64-v8a windows-x86_64
pause
goto :eof