#!/bin/bash

# declFlag flagName flagVar
declFlag() {

	for var in ${BASH_ARGV[*]}
	do
		if [ "$var" == "-$1" ]
		then
			eval $2=true
			return
		fi
	done

}

# declParam paramName paramVar
declParam() {

	for var in ${BASH_ARGV[*]}
	do
		if [[ "$var" = "-$1="* ]]
		then
			eval $2="${var/-$1=/}"
			return
		fi
	done

}

# Default params

env=all

declFlag release release
declFlag exclude_ext_formats exexfo
declFlag help helpMe
declFlag no_console noConsole
declFlag strip_debug_info strip
declFlag cmake cmake
declParam env env

if [ "$env" == "all" ] ; then
	env=x86,x64,ARM32,ARM64
fi

if [ $helpMe ]
then
	echo Windows requires the following dependencies to be installed:
	echo - Visual Studio
	echo - Vulkan SDK
	echo
	echo "Command line args:"
	echo "-cmake Reloads or initializes the CMake data"
	echo "-env=all Platform (x86, x64, ARM32, ARM64)"
	echo "-release Release environment (debug by default)"
	echo "-exclude_ext_formats Exclude external formats (only allow baked formats to be packaged; including pngs)"
	echo "-no_console Hides console (program can still redirect console calls)"
	echo "-strip_debug_info Strips debug info (shaders)"
	exit
fi

# reload "dir" "generator"
reload(){
	
	mkdir -p "$1"
	cd "$1"

	if [ $noConsole ] ; then
		params="-Dno_console=ON "
	fi
	
	if [ $strip ] ; then
		params="-Dstrip_debug_info=ON $params"
	fi
	
	cmake ../../../ -G "$2" $params
	cd ../

}

mkdir -p builds/Windows
cd builds/Windows

if [ $cmake ] ; then

	if [[ "$env" == *"x64"* ]] ; then
		reload "x64" "Visual Studio 15 2017 Win64"
	fi

	if [[ "$env" == *"x86"* ]] ; then
		reload "x86" "Visual Studio 15 2017"
	fi

	if [[ "$env" == *"ARM64"* ]] ; then
		reload "ARM64" "Visual Studio 15 2017 ARM64"
	fi

	if [[ "$env" == *"ARM32"* ]] ; then
		reload "ARM" "Visual Studio 15 2017 ARM"
	fi

fi

# Build type

if [ $release ] ; then
	btype="Release"
else
	btype="Debug"
fi

# Build

if [[ "$env" == *"x64"* ]] ; then
	cmd.exe /c "MSBuild.exe \"x64/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"x64\" /p:PostBuildEventUseInBuild=false"
fi

if [[ "$env" == *"x86"* ]] ; then
	cmd.exe /c "MSBuild.exe \"x86/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"Win32\" /p:PostBuildEventUseInBuild=false"
fi

if [[ "$env" == *"ARM64"* ]] ; then
	cmd.exe /c "MSBuild.exe \"ARM64/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"ARM64\" /p:PostBuildEventUseInBuild=false"
fi

if [[ "$env" == *"ARM32"* ]] ; then
	cmd.exe /c "MSBuild.exe \"ARM/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"ARM\" /p:PostBuildEventUseInBuild=false"
fi

rm -rf build/*
mkdir -p build

# Copy results

if [[ "$env" == *"x64"* ]] ; then
	cp "x64/bin/$btype/Osomi Core.exe" "build/Osomi Core x64.exe"
fi

if [[ "$env" == *"x86"* ]] ; then
	cp "x86/bin/$btype/Osomi Core.exe" "build/Osomi Core x86.exe"
fi

if [[ "$env" == *"ARM64"* ]] ; then
	cp "ARM64/bin/$btype/Osomi Core.exe" "build/Osomi Core ARM64.exe"
fi

if [[ "$env" == *"ARM32"* ]] ; then
	cp "ARM/bin/$btype/Osomi Core.exe" "build/Osomi Core ARM.exe"
fi

# Prepare resources

cd ../../app

if [ $strip ] ; then
	"../oibaker.exe" -strip_debug_info
else
	"../oibaker.exe"
fi

cd ../builds/Windows/build
mkdir -p res
cp -r ../../../app/res/* res
cd res

# Get rid of fbx, obj, oiBM and glsl/hlsl/vert/frag/geom/comp files

if [ $exexfo ]
then

	find . -type f -name '*.oiBM' -exec rm -f {} +
	find . -type f -name '*.fbx' -exec rm -f {} +
	find . -type f -name '*.obj' -exec rm -f {} +
	find . -type f -name '*.glsl' -exec rm -f {} +
	find . -type f -name '*.hlsl' -exec rm -f {} +
	find . -type f -name '*.vert' -exec rm -f {} +
	find . -type f -name '*.frag' -exec rm -f {} +
	find . -type f -name '*.geom' -exec rm -f {} +
	find . -type f -name '*.comp' -exec rm -f {} +

fi

cd ../../../