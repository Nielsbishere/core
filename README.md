# OCore (Osomi Core)
Osomi Core - A basic framework for graphics and engines.

# Build status & quality

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/dc530b2832e64edc8b2f320e0f38bf50)](https://www.codacy.com/app/niels_3/ocore?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OsomiLunar/ocore&amp;utm_campaign=Badge_Grade)

| Device        | Platform           | Status  |
| ------------- | ------------- | ----- |
| Windows      |  x86, x86_64 | [![Build status](https://ci.appveyor.com/api/projects/status/mtmreeiiey3y1ksd?svg=true)](https://ci.appveyor.com/project/OsomiLunar/ocore) |
| Android      | x86, x86_64, armeabi-v7a, arm64-v8a      |   [![Build Status](https://travis-ci.com/OsomiLunar/ocore.svg?branch=OCore_v2)](https://travis-ci.com/OsomiLunar/ocore) ||
| Linux | none      |    no |
| Mac OS | none      |    no |
# Fetching from git

For fetching this repo and dependencies:

```bat
git clone --recurse-submodules -j8 git://github.com/osomilunar/ocore.git
```

Or for already cloned repos:

```bat
git submodule update --init --recursive 
```

# Dependencies

The following dependencies are required for the entire project:

[CMake](https://cmake.org/download/) 3.6.0 or later
[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) 1.1.85.0 or later

### Windows

The project uses bash; Unix's built in scripting language. This means that you need to emulate Unix's functions and Unix Makefiles. This can be done by installing the [Git BASH](https://gitforwindows.org/) and [Mingw-w64](https://mingw-w64.org/). Make sure that you use the Git BASH client, not Cygwin's bash; to force this, you can use `bash -c "$cmd"` where cmd is your command and bash is Git BASH; either through path variable or an absolute path.

# Building using CMake

This project uses CMake and a few tools to help the user setup their environment.
## Setting up a Windows environment
### Dependencies

[Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2017)

### Environment variables

If you want to distribute the zip with x64 and x86 binaries, you require to have the msbuild command from vs17 available from command line. This means that you require to add the following path to your path environment variable:

`<vs17dir>/MSBuild/15.0/Bin`

Where vs17dir is the installation directory for Visual Studio 2017.

#### make_windows.sh

Packaging a Windows build will ensure that both x64 and x86 binaries are in the final zip. It will pull all required resources and put them next to the exe so it can run. This requires you to have access to msbuild.

```sh
# Don't package .fbx, .obj and .glsl/.hlsl/.vert/.frag./.comp/.geom files (only .oiSH and .oiRM)
# Release build without console (x64 & x86)
make_windows.sh -cmake -release -exclude_ext_formats -no_console

# Also strip debug info (so the shader source can't be reversed as easily)
# This saves disk space as well as securing the SPV-side of the oiSH file.
make_windows.sh -cmake -release -exclude_ext_formats -no_console -strip_debug_info

# Debug build; with all external formats & console & debug info (x64 & x86)
make_windows.sh -cmake

# Release build; with all external formats (x64 only; no console)
make_windows.sh -cmake -env=x64 -no_console

# Get more info about the command
make_windows.sh -help
```

This will generate a folder; builds/Windows/build, which includes the x86 and x64 binaries. If you include the `-cmake` flag, it will reload/initialize the CMake data; while adding `-run` will automatically run the project when it is done building. Don't include the CMake flag if you don't change the environment (you don't add source or include files or change build flags), since it will require the project to rebuild entirely or partly.

## Setting up an Android environment

### Dependencies

Android [SDK](https://developer.android.com/studio/install) & [NDK](https://developer.android.com/ndk/)  
[Apache ANT](https://ant.apache.org/bindownload.cgi)  
[Enable developer mode and USB debugging](https://android.gadgethacks.com/how-to/android-basics-enable-developer-options-usb-debugging-0161948/)

### Environment variables

Before you can use Android, you have to ensure that all variables are set correctly;

**ANDROID_NDK (ndk directory)**

### Setting up build env

```sh
# Setup a debug environment for all android ABIs, API lvl 24, windows environment
make_android.sh -abi=windows-x86_64

# Setup for linux environment
make_android.sh

# Release environment for windows; with only .oiRM models, .oiSH shaders, textures and settings
make_android.sh -abi=windows-x86_64 -release -exclude_ext_formats

# Exclude debug info from shaders
make_android.sh -strip_debug_info

# Only build arm64-v8a debug for linux-x86_64
make_android.sh -abi=arm64-v8a

# Get more info about the command
make_android.sh -help
```
On Linux, this might require you to `chmod +x make_android.sh` before you use it.

#### Building / running apk

```sh
# Build APK (builds/Android/apk/bin)
build_android.sh

# Build and run APK
run_android.sh
```

Building an APK file requires you to build ALL architectures; arm64-v8a, armeabi-v7a, x86_64, x86. This means long compile times. Try to pick the ABI of your choosing (check your emulator or phone) and set the environment to build for that.

On Linux, this might require you to `chmod +x build_android.sh` and `chmod +x run_android.sh`.

## Baking all resources

If you want to bake the resources of your project (to get native resources), you can use the prepare_resources command:

```bat
# script
prepare_resources

# manual
cd app
"../oibaker"
cd ../
```

oibaker compiles GLSL/HLSL files into oiSH (SPIRV and reflection) and fbx/obj to oiRM.

**Note: oibaker is currently only available on Windows; but the baked resources are already uploaded to git.**

# How to use ocore in a project

If you want to use Osomi core for your project, you can go to your repo root directory. From there, you can add this repo as a dependency:
```bat
git submodule add https://github.com/osomilunar/ocore ocore
git submodule update --init --recursive 
```
This will fetch ocore into your project dir. Now you can include the subdirectory in CMake.

Please do yourself and us a favor and don't push this directory onto your repo, but instead use the submodule, so everyone gets ocore using our repo.

If you want to use ocore cross platform, it is recommended that you use the app entrypoints (app_pc, app_android, etc.).
# Getting started
There is documentation on [Osomi Graphics Core](docs/ogc.md) (ogc; rendering), [Osomi Window Core](docs/owc.md) (owc; window/app and input), [Osomi STandard Library Core](docs/ostlc.md) (ostlc; main data types and utils) and the top level entry [app](docs/app.md).

There's also documentation about the file formats used; [oiSL](docs/oiSL.md) (String List), [oiSB](docs/oiSB.md) (Shader Buffer), [oiSH](docs/oiSH.md) (SHader), [oiRM](docs/oiRM.md) (Raw Model), [oiBM](docs/oiBM.md) (BakeManager).

# Viewing progress
You can view progress and planned features on [our trello board](https://trello.com/b/US4bChrI/osomi-core).
