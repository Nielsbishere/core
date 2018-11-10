echo Android requires the following dependencies to be installed:
echo - Java
echo - Android SDK
echo - Android NDK set up as environment variable ANDROID_NDK
echo - MinGW Makefiles 64-bit
echo - Apache Ant
echo - Vulkan SDK

# Dev env, API lvl, ABI
makeAndroid() {

	echo Making Android build $1 $2 $3

	mkdir -p builds
	mkdir -p builds/Android
	mkdir -p builds/Android/$3
	cd builds/Android/$3

	cmake "../../../" -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-$2 -DCMAKE_MAKE_PROGRAM=${ANDROID_NDK}/prebuilt/$1/bin/make -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="$3" -DAndroid=ON -DANDROID_STL=c++_shared -DANDROID_APK_RUN=OFF
	
	echo make -j > run_android.sh
	cd ../../../

}

if [ "$1" == '' ]
then
	dev="linux-x86_64"
else
	dev="$1"
fi

if [ "$2" == '' ]
then
	lvl="24"
else
	lvl="$2"
fi

if [ "$3" == '' ]
then

	echo Creating builds for every Android ABI...
	echo For a specialized build: run_android $dev $lvl arm64-v8a

	abi="arm64-v8a"
	makeAndroid $dev $lvl $abi

	abi="armeabi-v7a"
	makeAndroid $dev $lvl $abi

	abi="x86"
	makeAndroid $dev $lvl $abi

	abi="x86_64"
	makeAndroid $dev $lvl $abi

else
	abi="$3"
	makeAndroid $dev $lvl $abi
fi
