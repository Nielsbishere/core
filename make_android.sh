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
abi=all
lvl=24
dev=linux-x86_64
gen=Unix\ Makefiles

# Params and flags
declFlag release release
declFlag exclude_ext_formats exexfo
declFlag help helpMe
declFlag strip_debug_info strip
declParam abi abi
declParam lvl lvl
declParam dev dev

if [ "$dev" == "windows-x86_64" ]
then
	gen=MinGW\ Makefiles
	makeCmd=mingw32-make
	android_ndk=${ANDROID_NDK}
	android_ndk="${android_ndk//\\//}"
else
	makeCmd=make
	android_ndk=${ANDROID_NDK}
fi

# Help

if [ $helpMe ]
then
	echo Android requires the following dependencies to be installed:
	echo - Java
	echo - Android SDK
	echo - Android NDK set up as environment variable ANDROID_NDK
	echo - MinGW Makefiles 64-bit
	echo - Apache Ant
	echo - Vulkan SDK
	echo
	echo "Command line args:"
	echo "-abi=all Android ABI (all if not specified)"
	echo "-lvl=24 Android API level (24 or higher)"
	echo "-dev=linux-x86_64 Dev environment (linux-x86_64 by default)"
	echo "-release Release environment (debug by default)"
	echo "-exclude_ext_formats Exclude external formats (only allow baked formats to be packaged; including pngs)"
	echo "-strip_debug_info Strips debug info (shaders)"
	exit
fi

# build abi
build(){

	echo "Making Android build ($dev $lvl $1)"
	
	mkdir -p builds
	mkdir -p builds/Android
	mkdir -p builds/Android/$1
	cd builds/Android/$1
	
	cmake "../../../" -G "$gen" -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-$lvl -DCMAKE_MAKE_PROGRAM=${ANDROID_NDK}/prebuilt/$dev/bin/make -DANDROID_ABI="$1" -DAndroid=ON -DANDROID_STL=c++_shared
	
	cd ../../../

}


if [ "$abi" == "all" ]
then

	# Make all builds

	build arm64-v8a
	build x86_64
	build armeabi-v7a
	build x86

	# Build all targets

	echo "#!/bin/bash" > build_android.sh
	echo cd builds/Android/arm64-v8a >> build_android.sh
	echo $makeCmd -d -j >> build_android.sh

	echo cd ../x86_64 >> build_android.sh
	echo $makeCmd -d -j >> build_android.sh

	echo cd ../armeabi-v7a >> build_android.sh
	echo $makeCmd -d -j >> build_android.sh

	echo cd ../x86 >> build_android.sh
	echo $makeCmd -d -j >> build_android.sh

	echo cd ../ >> build_android.sh

	# Make apk dirs

	echo rm -rf build >> build_android.sh
	echo mkdir -p build >> build_android.sh
	echo mkdir -p build/libs >> build_android.sh
	echo mkdir -p build/libs/arm64-v8a >> build_android.sh
	echo mkdir -p build/libs/x86_64 >> build_android.sh
	echo mkdir -p build/libs/armeabi-v7a >> build_android.sh
	echo mkdir -p build/libs/x86 >> build_android.sh

	# Dependencies

	if ! [ $release ]
	then
		echo cp -r $android_ndk/sources/third_party/vulkan/src/build-android/jniLibs/* build/libs >> build_android.sh
		echo rm -rf build/libs/mips >> build_android.sh
		echo rm -rf build/libs/mips64 >> build_android.sh
	fi

	echo cp -r $android_ndk/sources/cxx-stl/llvm-libc++/libs/* build/libs >> build_android.sh

	# Prepare assets, src and AndroidManifest, build.xml

	echo cp -r arm64-v8a/bin/build/* build >> build_android.sh

else

	# Make build
	build $abi
	
	# Build all targets

	echo cd builds/Android/$abi > build_android.sh
	echo $makeCmd -d -j >> build_android.sh
	echo cd ../ >> build_android.sh

	# Make apk dirs

	echo rm -rf build >> build_android.sh
	echo mkdir -p build >> build_android.sh
	echo mkdir -p build/libs >> build_android.sh
	echo mkdir -p build/libs/$abi >> build_android.sh

	# Dependencies

	if ! [ $release ]
	then
		echo cp -r $android_ndk/sources/third_party/vulkan/src/build-android/jniLibs/$abi/* build/libs/$abi >> build_android.sh
	fi

	echo cp -r $android_ndk/sources/cxx-stl/llvm-libc++/libs/$abi/* build/libs/$abi >> build_android.sh

	# Prepare src and AndroidManifest, build.xml

	echo cp -r $abi/bin/build/* build >> build_android.sh

fi

# Prepare assets and src

echo mkdir -p build/src >> build_android.sh
echo cp -r ../../app_android/src/* build/src >> build_android.sh
echo cp -r ../../app_android/res/* build/res >> build_android.sh

echo mkdir -p build/assets >> build_android.sh
echo mkdir -p build/assets/res >> build_android.sh

# Run baker

if [ "$dev" == "windows-x86_64" ] ; then

	echo cd ../../app >> build_android.sh
	
	if [ $strip ] ; then
		echo "../oibaker.exe" -strip_debug_info >> build_android.sh
	else
		echo "../oibaker.exe" >> build_android.sh
	fi

	echo cd ../builds/Android >> build_android.sh

fi

# Copy results

echo cp -r ../../app/res/* build/assets/res >> build_android.sh

# Filter out some extensions

if [ $exexfo ]
then
	echo cd build/assets/res >> build_android.sh
	echo find . -type f -name '*.oiBM' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.fbx' -exec rm -f {} + >> build_android.sh
	echo find . -type f -name '*.obj' -exec rm -f {} + >> build_android.sh
	echo find . -type f -name '*.glsl' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.hlsl' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.vert' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.frag' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.geom' -exec rm -f {} +	>> build_android.sh
	echo find . -type f -name '*.comp' -exec rm -f {} +	>> build_android.sh
	echo cd ../../../ >> build_android.sh
fi

if [ "$abi" == "all" ]
then

	# Copy build results

	echo cp arm64-v8a/lib/libapp_android.so build/libs/arm64-v8a/libapp_android.so >> build_android.sh
	echo cp x86_64/lib/libapp_android.so build/libs/x86_64/libapp_android.so >> build_android.sh
	echo cp armeabi-v7a/lib/libapp_android.so build/libs/armeabi-v7a/libapp_android.so >> build_android.sh
	echo cp x86/lib/libapp_android.so build/libs/x86/libapp_android.so >> build_android.sh

else

	# Copy build results

	echo cp $abi/lib/libapp_android.so build/libs/$abi/libapp_android.so >> build_android.sh

fi

echo cd build >> build_android.sh

if [ $release ]
then
	echo ant release >> build_android.sh
	# echo jarsigner -verbose -keystore ~/my-release-key.keystore bin/app_android-unsigned.apk myalias >> build_android.sh
	echo zipalign -v -f 4 bin/app_android-unsigned.apk bin/app_android.apk >> build_android.sh
else
	echo ant debug >> build_android.sh
fi

# run script
echo "#!/bin/bash" > run_android.sh
echo ./build_android.sh >> run_android.sh
echo cd builds/Android/build/bin >> run_android.sh

if [ $release ]
then
	echo adb install -r app_android.apk >> run_android.sh
else
	echo adb install -r app_android-debug.apk >> run_android.sh
fi

echo adb shell am start -n net.osomi.Osomi_Core/android.app.NativeActivity >> run_android.sh
echo adb logcat -c >> run_android.sh
echo adb logcat -s "oi_Log" >> run_android.sh

cd ../../