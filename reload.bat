@echo off

if "%~1"=="" goto :reg

mkdir "builds/%~1"
cd "builds/%~1"
cmake ../../ -G "%~1"
cd "%~dp0"
goto :eof

:reg
echo Please supply the generator to reload