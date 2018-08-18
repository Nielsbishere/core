@echo off

if NOT "%~1"=="" (
	echo Creating project using generator "%~1"
	reload "%~1"
	goto :eof
)

reload "Visual Studio 15 2017 Win64" & reload "Visual Studio 15 2017"