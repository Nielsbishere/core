@echo off

echo Creating ocore project at %cd% with template %~dp0

xcopy "%~dp0CMakeLists.project.txt" "%cd%/CMakeLists.txt*" /y >nul

if not exist "%cd%/app" mkdir "%cd%/app"
xcopy "%~dp0app" "%cd%/app" /i /y /s >nul
del "%cd%\app\CMakeLists.txt" >nul
ren "%cd%\app\CMakeLists.project.txt" "CMakeLists.txt" >nul

if not exist "%cd%/app_android" mkdir "%cd%/app_android"
xcopy "%~dp0app_android" "%cd%/app_android" /i /y /s >nul
del "%cd%\app_android\CMakeLists.txt" >nul
ren "%cd%\app_android\CMakeLists.project.txt" "CMakeLists.txt" >nul

if not exist "%cd%/app_pc" mkdir "%cd%/app_pc"
xcopy "%~dp0app_pc" "%cd%/app_pc" /i /y /s >nul
del "%cd%\app_pc\CMakeLists.txt" >nul
ren "%cd%\app_pc\CMakeLists.project.txt" "CMakeLists.txt" >nul

update_project