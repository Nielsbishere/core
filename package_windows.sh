#!/bin/bash

# simple pause
pause() {
	read -p "Press enter to continue..."
	exit
}

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
declParam env env

if [ $helpMe ]
then
	echo Windows requires the following dependencies to be installed:
	echo - Visual Studio
	echo - Vulkan SDK
	echo
	echo "Command line args:"
	echo "- -env=all Platform (x86, x64)"
	echo "- -release Release environment (debug by default)"
	echo "- -exclude_ext_formats Exclude external formats (only allow baked formats to be packaged; including pngs)"
	pause
fi

./make_pc.sh

mkdir -p builds/Windows
cd builds/Windows

# Multi core compiling & minimal verbosity

params="/m /v:m"
btype="Debug"

if [ $release ]
then
	params="$params /p:Configuration=Release"
	btype="Release"
fi

# Build

if [ "$env" == "all" ] || [ "$env" == "x64" ] ; then
	cmd.exe /c "MSBuild.exe \"Visual Studio 15 2017 Win64/oic.sln\" $params /p:Platform=\"x64\""
fi

if [ "$env" == "all" ] || [ "$env" == "x86" ] ; then
	cmd.exe /c "MSBuild.exe \"Visual Studio 15 2017/oic.sln\" $params /p:Platform=\"x86\""
fi

rm -rf build
mkdir -p build

# Copy results

if [ "$env" == "all" ] || [ "$env" == "x64" ] ; then
	cp "Visual Studio 15 2017 Win64/bin/$btype/Osomi Core.exe" "build/Osomi Core.exe"
fi

if [ "$env" == "all" ] || [ "$env" == "x86" ] ; then
	cp "Visual Studio 15 2017/bin/$btype/Osomi Core.exe" "build/Osomi Core x86.exe"
fi

# Copy resources

mkdir -p build/res

if [ $exexfo ]
then

	mkdir -p build/res/models
	mkdir -p build/res/shaders
	mkdir -p build/res/textures
	mkdir -p build/res/settings

	find ../../app/res/models -name \*.oiRM -exec cp {} build/res/models \;
	find ../../app/res/models -name \*.json -exec cp {} build/res/models \;
	find ../../app/res/shaders -name \*.oiSH -exec cp {} build/res/shaders \;
	cp -r ../../app/res/textures/* build/res/textures
	cp -r ../../app/res/settings/* build/res/settings

else
	cp -r ../../app/res/* build/res
fi

cd ../../