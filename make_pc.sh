#!/bin/bash

# reload "Generator"
reload(){
	
	mkdir -p "builds/Windows"
	mkdir -p "builds/Windows/$1"
	cd "builds/Windows/$1"
	cmake ../../../ -G "$1"
	cd ../../../

}

if ! [ "$1" == "" ]
then
	echo "Creating project using generator $1"
	reload "$1"
else
	reload "Visual Studio 15 2017 Win64"
	reload "Visual Studio 15 2017"
fi