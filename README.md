# OCore (Osomi Core)
Osomi Core - A basic framework for engines  
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/dc530b2832e64edc8b2f320e0f38bf50)](https://www.codacy.com/app/niels_3/ocore?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OsomiLunar/ocore&amp;utm_campaign=Badge_Grade)
# Build status
| Device        | Platform           | Status  |
| ------------- | ------------- | ----- |
| Windows      |  x86, x86_64 | [![Build status](https://ci.appveyor.com/api/projects/status/mtmreeiiey3y1ksd?svg=true)](https://ci.appveyor.com/project/OsomiLunar/ocore) |
| Android      | x86, x86_64, armeabi-v7a, arm64-v8a      |   no ||
| Linux | none      |    no |
| Mac OS | none      |    no |  
# Building using CMake
This project uses CMake and a few tools to help the user setup their environment.
## Cleaning & Reloading project
The user can run the 'clean' script to either regenerate or get rid of all generated cmake files. Clean without arguments gets rid of them, and with arguments runs reload. Reload either reloads the project or reinstalls it, depending on if it has been run before.
```bat
rem Clean the CMake and generated files
clean.bat
rem Clean the specified build directory
clean.bat "MinGW Makefiles"
rem Reload with a specified generator (desktop)
reload.bat "MinGW Makefiles"
```
## Building for Android
If you have developer mode enabled on your Android device and have the Android NDK version that supports the 'android update' command, you can build it as long as the environment variables are set correctly. You can create the MinGW makefile by using 'make_android.bat' (if the project is clean) and turn it into a .apk and run it by using 'run_android.bat'.
```bat
# Setup an Android CMake project; with default settings (API level 24, arm64-v8a/armeabi-v7a/x86/x64 architecture and windows-x86_64 environment)
make_android.bat
# Setup an Android CMake project; specified for the ABI (if you leave it out, it will create 4 different ABI directories)
make_android.bat windows-x86_64 24 arm64-v8a
# Run android (requires connected device with developer settings)
"builds/Android %abi% %lvl%/run_android.bat"
```
## Building for Windows
'make_pc.bat' creates a Visual Studio project, which means you need Visual Studio. If you don't want to use Visual Studio, you can specify a different generator.
```bat
# Setup a Visual Studio 15 x86_64 and x86 project
make_pc.bat
# Explicitly set the generator
make_pc.bat "Visual Studio 15 2017 Win64"
```
## Compiling shaders
Normally, this is done automatically when you compile your project. However, if you don't want to recompile or can't, you can still convert the shaders to oiSH (Osomi SHader) file format.
```bat
compile_shaders.bat
```
## Baking models
Using oiRM (Osomi Raw Model) files is way easier & faster than using fbx/obj files. This is why you have to convert models to this format; to avoid long load times. This can be done by going into the app directory and running bake_models. Just like compile_shaders, this is run automatically by the build system.
```bat
cd app
"../bake_models.bat"
cd ../
```
# How to use ocore in a project
If you want to use Osomi core for your project, you can go to your repo root directory. From there, you can add this repo as a dependency:
```bat
git submodule add https://github.com/osomilunar/ocore ocore
```
This will fetch ocore into your project dir. It contains CMake files you can add to your own, however, it is recommended that you use all of the ocore.
Please do yourself and us a favor and don't push this directory onto your repo, but instead push the submodule, so everyone gets ocore using our repo.
# How to create a new project using all of ocore
If you want to use all of ocore, but you want to create your own app or library, you can use the following command:
```bat
"ocore/make_project"
```
Which will copy all required cmake files, project files, etc. to your project.
# Getting started
To understand how to use Osomi Graphics Core (ogc; rendering), Osomi Window Core (owc; window/app and input), Osomi STandard Library Core (ostlc; main data types and utils), you can look at the docs directory, which contains markdown files about every important feature of the library.
# Viewing progress
You can view progress and planned features on [our trello board](https://trello.com/b/US4bChrI/osomi-core).
