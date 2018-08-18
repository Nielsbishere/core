@echo off

rem clean bin & lib
del /s /f /q bin 2>nul
del /s /f /q lib 2>nul

if "%~1"=="" goto :other

rem clean specific build

del /s /f /q "builds/%~1" 2>nul
rd /s /q "builds/%~1" 2>nul
mkdir "builds/%~1"
goto :eof

:other

rem clean all builds

del /s /f /q builds 2>nul
rd /s /q builds 2>nul
mkdir builds