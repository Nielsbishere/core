# OCore (Osomi Core)
Osomi Core - A basic framework for graphics and engines.

# Build status & quality

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/dc530b2832e64edc8b2f320e0f38bf50)](https://www.codacy.com/app/niels_3/ocore?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=OsomiLunar/ocore&amp;utm_campaign=Badge_Grade)

| Device        | Platform           | Status  |
| ------------- | ------------- | ----- |
| Windows      |  x86, x86_64 | [![Build status](https://ci.appveyor.com/api/projects/status/mtmreeiiey3y1ksd?svg=true)](https://ci.appveyor.com/project/OsomiLunar/ocore) |
| Android      | x86, x86_64, armeabi-v7a, arm64-v8a      |   no ||
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

[CMake](https://cmake.org/download/) 3.9.0 or later

[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) 1.1.85.0 or later

# Building using CMake

This project uses CMake and a few tools to help the user setup their environment.
## Setting up a Windows environment
### Dependencies

[Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2017) or [Mingw-w64](https://mingw-w64.org/)

### make_pc

```bat
# Visual Studio 15 2017 and Win64 (x86 and x86_64)
make_pc
# Mingw-w64
make_pc "MinGW Makefiles"
```
## Setting up an Android environment

### Dependencies

Android [SDK](https://developer.android.com/studio/install) & [NDK](https://developer.android.com/ndk/)
[Mingw-w64](https://mingw-w64.org/)
[Apache ANT](https://ant.apache.org/bindownload.cgi)
[Enable developer mode and USB debugging](https://android.gadgethacks.com/how-to/android-basics-enable-developer-options-usb-debugging-0161948/)

### Environment variables

Before you can use Android, you have to ensure that all variables are set correctly;

JAVA_HOME (jdk directory)
ANDROID_HOME (sdk directory)
ANDROID_NDK_HOME (ndk directory)

Now you can build for Android

```bat
# Setup an Android CMake project; with default settings (API level 24, arm64-v8a/armeabi-v7a/x86/x64 architecture and windows-x86_64 environment)
make_android.bat

# Setup an Android CMake project; specified for the ABI (if you leave it out, it will create 4 different ABI directories)
make_android.bat windows-x86_64 24 arm64-v8a

# Run app on Android (requires connected device with developer settings)
"builds/Android/%abi%/run_android.bat"
```
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
