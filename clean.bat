@echo off
del /s CMakeCache.txt
del /s Makefile
del /s cmake_install.cmake
del /s *.sln
del /s *.vcxproj
del /s *.filters
del /s *.vcxproj.user
rd /s /q CMakeFiles
rd /s /q ostlc\CMakeFiles
rd /s /q ostlc\ostlc.dir
rd /s /q owc\CMakeFiles
rd /s /q owc\owc.dir
rd /s /q app\CMakeFiles
rd /s /q app\app.dir
rd /s /q app_pc\CMakeFiles
rd /s /q app_pc\app_pc.dir
rd /s /q app_android\CMakeFiles
rd /s /q app_android\app_android.dir
rd /s /q %~dp0\lib
rd /s /q %~dp0\bin

if "%~1"=="" goto :eof

reload.bat "%~1"