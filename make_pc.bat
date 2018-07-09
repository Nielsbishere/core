@echo off

if NOT "%~1"=="" (
	echo Creating project using generator "%~1"
	clean "%~1"
	goto :eof
)

clean "Visual Studio 15 2017 Win64"