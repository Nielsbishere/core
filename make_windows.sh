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

# If latest command failed; quit
checkErrors() {

	if ! [ $? == 0 ] ; then
		exit 1
	fi

}

# Default params

env=all

declFlag release release
declFlag exclude_ext_formats exexfo
declFlag help helpMe
declFlag no_console noConsole
declFlag strip_debug_info strip
declFlag cmake cmake
declFlag shader_compilation shcmp
declFlag enable_oibaker enoib
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
	echo "-shader_compilation Allows runtime shader compilation"
	echo "-enable_oibaker Enables oibaker (for baking resources; requires runtime shader compilation)"
	exit
fi

# reload "dir" "generator"
reload(){
	
	mkdir -p "$1"
	cd "$1"
	
	cmake ../../../ -G "$2" $params
	checkErrors
	cd ../

}

# Prepare resources

checkErrors


# Build type

if [ $release ] ; then
	btype="Release"
else
	btype="Debug"
fi

# Make build

mkdir -p builds/Windows
cd builds/Windows

params=-DCMAKE_BUILD_TYPE=$btype

if [ $noConsole ] ; then
	params="-Dno_console=ON $params"
else
	params="-Dno_console=OFF $params"
fi

if [ $strip ] ; then
	params="-Dstrip_debug_info=ON $params"
else
	params="-Dstrip_debug_info=OFF $params"
fi

if [ $exexfo ] ; then
	params="-Dexclude_ext_formats=ON $params"
else
	params="-Dexclude_ext_formats=OFF $params"
fi

if [ $shcmp ] ; then
	params="-Dshader_compilation=ON $params"
else
	params="-Dshader_compilation=OFF $params"
fi

if [ $enoib ] ; then
	params="-Denable_oibaker=ON $params"
else
	params="-Denable_oibaker=OFF $params"
fi

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

# Build

if [[ "$env" == *"x64"* ]] ; then
	cmd.exe /c "MSBuild.exe \"x64/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"x64\" /p:PostBuildEventUseInBuild=false"
	checkErrors
fi

if [[ "$env" == *"x86"* ]] ; then
	cmd.exe /c "MSBuild.exe \"x86/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"Win32\" /p:PostBuildEventUseInBuild=false"
	checkErrors
fi

if [[ "$env" == *"ARM64"* ]] ; then
	cmd.exe /c "MSBuild.exe \"ARM64/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"ARM64\" /p:PostBuildEventUseInBuild=false"
	checkErrors
fi

if [[ "$env" == *"ARM32"* ]] ; then
	cmd.exe /c "MSBuild.exe \"ARM/oic.sln\" /m /v:m /p:Configuration=$btype /p:Platform=\"ARM\" /p:PostBuildEventUseInBuild=false"
	checkErrors
fi

rm -rf build/*
mkdir -p build

# Copy results

if [[ "$env" == *"x64"* ]] ; then
	cp "x64/bin/$btype/app_windows.exe" "build/app_windows x64.exe"
fi

if [[ "$env" == *"x86"* ]] ; then
	cp "x86/bin/$btype/app_windows.exe" "build/app_windows x86.exe"
fi

if [[ "$env" == *"ARM64"* ]] ; then
	cp "ARM64/bin/$btype/app_windows.exe" "build/app_windows ARM64.exe"
fi

if [[ "$env" == *"ARM32"* ]] ; then
	cp "ARM/bin/$btype/app_windows.exe" "build/app_windows ARM.exe"
fi