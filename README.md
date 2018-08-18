# OCore (Osomi Core)
Osomi Core - A basic template library for (graphic) engines
## Building using CMake
This project uses CMake and a few tools to help the user setup their environment.
### Cleaning & Reloading project
The user can run the 'clean' script to either regenerate or get rid of all generated cmake files. Clean without arguments gets rid of them, and with arguments runs reload. Reload either reloads the project or reinstalls it, depending on if it has been run before.
```bat
# Clean the CMake and generated files
clean.bat
# Clean the project and make it as a MinGW Makefiles project
clean.bat "MinGW Makefiles"
# Reload the project
reload.bat
```
### Building for Android
If you have developer mode enabled on your Android device and have the Android NDK version that supports the 'android update' command, you can build it as long as the environment variables are set correctly. You can create the MinGW makefile by using 'make_android.bat' (if the project is clean) and turn it into a .apk and run it by using 'run_android.bat'.
```bat
# Setup an Android CMake project; with default settings (API level 24, arm64-v8a architecture and windows-x86_64 environment)
make_android.bat
# Setup an Android CMake project; with custom settings (but same as before)
make_android.bat windows-x86_64 arm64-v8a 24
# Run android (requires connected device with developer settings)
run_android.bat
```
### Building for Windows
'make_pc.bat' creates a Visual Studio project, which means you need Visual Studio. If you don't want to use Visual Studio, you can specify a different generator.
```bat
# Setup a Visual Studio 15 x86_64 project
make_pc.bat
# Explicitly set the generator
make_pc.bat "Visual Studio 15 2017 Win64"
```
### Compiling shaders
Normally, this is done automatically when you compile your project. However, if you don't want to recompile or can't, you can still convert the shaders to oiSH (Osomi SHader) file format.
```bat
compile_shaders.bat
```
