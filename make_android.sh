#!/bin/bash

# declFlag flagName flagVar
declFlag() {

	for var in ${BASH_ARGV[*]}
	do
		if [ "$var" == "-$1" ] ; then
			eval $2=true
			return
		fi
	done

}

# declParam paramName paramVar
declParam() {

	for var in ${BASH_ARGV[*]}
	do
		if [[ "$var" = "-$1="* ]] ; then
			eval $2="${var/-$1=/}"
			return
		fi
	done

}

# Default params
abi=all
lvl=24
dev=linux-x86_64

# Params and flags
declFlag release release
declFlag exclude_ext_formats exexfo
declFlag help helpMe
declFlag strip_debug_info strip
declFlag apk apk
declFlag run run
declFlag cmake cmake
declParam abi abi
declParam lvl lvl
declParam dev dev
declParam jobs jobs

if [ "$abi" == "all" ] ; then
	abi=x86,x64,ARM32,ARM64
fi

if [ "$dev" == "windows-x86_64" ] ; then
	gen=MinGW\ Makefiles
	makeCmd=mingw32-make
	android_ndk=${ANDROID_NDK}
	android_ndk="${android_ndk//\\//}"
else
	makeCmd=make
	android_ndk=${ANDROID_NDK}
fi

# Help

if [ $helpMe ] ; then
	echo Android requires the following dependencies to be installed:
	echo - Java
	echo - Android SDK
	echo - Android NDK set up as environment variable ANDROID_NDK
	echo - MinGW Makefiles 64-bit
	echo - Vulkan SDK
	echo
	echo "Command line args:"
	echo "-cmake Reloads or initializes the CMake data"
	echo "-apk Creates apk file; otherwise only compiles"
	echo "-run Runs the build"
	echo "-abi=all Android ABI (ARM64, ARM32, x64, x86)"
	echo "-lvl=24 Android API level (>=24)"
	echo "-dev=linux-x86_64 Dev environment (linux-x86_64, windows-x86_64)"
	echo "-release Release environment (debug by default)"
	echo "-exclude_ext_formats Exclude external formats (only allow baked formats to be packaged)"
	echo "-strip_debug_info Strips debug info (shaders)"
	echo "-jobs=2 To set the max compilation jobs for parallel compilation"
	exit
fi

# cmake abi

build(){

	if [ $cmake ] ; then

		echo "Making Android build ($dev $lvl $1)"
		
		mkdir -p builds
		mkdir -p builds/Android
		mkdir -p builds/Android/$1
		cd builds/Android/$1
		
		if ! [ "$gen" == "" ] ; then
			cmake "../../../" -G "$gen" -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}/build/cmake/android.toolchain.cmake" -DANDROID_NATIVE_API_LEVEL=android-$lvl -DCMAKE_MAKE_PROGRAM="${ANDROID_NDK}/prebuilt/$dev/bin/make" -DANDROID_ABI="$1" -DAndroid=ON -DANDROID_STL=c++_shared $cmakeParams
		else
			cmake "../../../" -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}/build/cmake/android.toolchain.cmake" -DANDROID_NATIVE_API_LEVEL=android-$lvl -DCMAKE_MAKE_PROGRAM="${ANDROID_NDK}/prebuilt/$dev/bin/make" -DANDROID_ABI="$1" -DAndroid=ON -DANDROID_STL=c++_shared $cmakeParams
		fi
	
	
		cd ../../../
	
	fi

}

# build abi

buildAbi(){

	if [ $1 == "ARM32" ] ; then
		d="armeabi-v7a"
	elif [ $1 == "ARM64" ] ; then
		d="arm64-v8a"
	elif [ $1 == "x64" ] ; then
		d="x86_64"
	elif [ $1 == "x86" ] ; then
		d="x86"
	fi

	# Make build

	build "$d"
	cd builds/Android/$d
	eval "$makeCmd" $params
	cd ../
	
	mkdir -p build/lib/$d
	
	# Dependencies (shared/dynamic)

	if ! [ $release ] ; then
		cp -r $android_ndk/sources/third_party/vulkan/src/build-android/jniLibs/$d/* build/lib/$d
	fi

	cp -r $android_ndk/sources/cxx-stl/llvm-libc++/libs/$d/libc++_shared.so build/lib/$d

	cd ../../

}

# CMake & build

if ! [ $jobs ] ; then
	params="-j"
else
	params="-j$jobs"
fi

rm -rf build
mkdir -p build
mkdir -p build/lib

for i in $(echo $abi | sed "s/,/ /g")
do
    buildAbi "$i"
done

# Prepare assets, src and AndroidManifest, build.xml

firstAbi=$(echo $abi | cut -d, -f1)
	
if [ $firstAbi == "ARM32" ] ; then
	d="armeabi-v7a"
elif [ $firstAbi == "ARM64" ] ; then
	d="arm64-v8a"
elif [ $firstAbi == "x64" ] ; then
	d="x86_64"
elif [ $firstAbi == "x86" ] ; then
	d="x86"
fi

cd builds/Android
cp -r $d/bin/build/* build

# Prepare assets and src

mkdir -p build/src
cp -r ../../app_android/src/* build/src
cp -r ../../app_android/res/* build/res

mkdir -p build/assets
mkdir -p build/assets/res

# Run baker

if [ "$dev" == "windows-x86_64" ] ; then

	cd ../../app
	
	if [ $strip ] ; then
		"../oibaker.exe" -strip_debug_info
	else
		"../oibaker.exe"
	fi

	cd ../builds/Android

fi

# Copy results

if [ $apk ] ; then

	cp -r ../../app/res/* build/assets/res
	
	# Filter out some extensions
	
	if [ $exexfo ]
	then
		cd build/assets/res
		find . -type f -name '*.oiBM' -exec rm -f {} +
		find . -type f -name '*.fbx' -exec rm -f {} +
		find . -type f -name '*.obj' -exec rm -f {} +
		find . -type f -name '*.glsl' -exec rm -f {} +
		find . -type f -name '*.hlsl' -exec rm -f {} +
		find . -type f -name '*.vert' -exec rm -f {} +
		find . -type f -name '*.frag' -exec rm -f {} +
		find . -type f -name '*.geom' -exec rm -f {} +
		find . -type f -name '*.comp' -exec rm -f {} +
		cd ../../../
	fi
	
	for i in $(echo $abi | sed "s/,/ /g")
	do
	
		d=none
	
		if [ $i == "ARM32" ] ; then
			d="armeabi-v7a"
		elif [ $i == "ARM64" ] ; then
			d="arm64-v8a"
		elif [ $i == "x64" ] ; then
			d="x86_64"
		elif [ $i == "x86" ] ; then
			d="x86"
		fi
		
		if ! [ $d == "none" ] ; then
			cp $d/lib/libapp_android.so build/lib/$d
		fi
	done

	cd build
	
	# Create APK and add libs
	
	mkdir -p bin
	
	aapt package -f -I "${ANDROID_SDK}/platforms/android-$lvl/android.jar" -M AndroidManifest.xml -A assets -S res -m -F bin/app-unsigned.apk
	
	libs=$(find . -type f -name '*.so' | sed 's/\\/\//g')
	
	for lib in $(echo ${libs//.\//} | tr " " "\n")
	do
		aapt add bin/app-unsigned.apk $lib
	done
	
	# Align and sign APK
	
	cd bin
	zipalign -v -f 4 app-unsigned.apk app.apk
	
	if ! [ -e "../../.keystore" ] ; then
		"${JAVA_HOME}/bin/keytool" -genkeypair -v -keystore ../../.keystore -keyalg RSA -keysize 2048 -validity 10000
	fi
	
	if [ "$dev" == "windows-x86_64" ] ; then
		apksigner.bat sign --ks ../../.keystore --min-sdk-version 24 app.apk
	else
		apksigner.sh sign --ks ../../.keystore --min-sdk-version 24 app.apk
	fi

	cd ../
	
fi
	
# run script

cd bin

if [ $run ] ; then
	adb install -r app.apk
	adb shell am start -n net.osomi.Osomi_Core/android.app.NativeActivity
	adb logcat -c
	adb logcat -s "oi_Log"
fi

cd ../../../../