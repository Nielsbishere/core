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

xcopy "%~dp0clean.bat" "%cd%/clean.bat*" /y >nul
xcopy "%~dp0reload.bat" "%cd%/reload.bat*" /y >nul
xcopy "%~dp0compile_shaders.bat" "%cd%/compile_shaders.bat*" /y >nul
xcopy "%~dp0bake_models.bat" "%cd%/bake_models.bat*" /y >nul
xcopy "%~dp0prepare_resources.bat" "%cd%/prepare_resources.bat*" /y >nul
xcopy "%~dp0make_android.bat" "%cd%/make_android.bat*" /y >nul
xcopy "%~dp0make_pc.bat" "%cd%/make_pc.bat*" /y >nul
xcopy "%~dp0oirm_gen.exe" "%cd%/oirm_gen.exe*" /y >nul
xcopy "%~dp0oish_gen.exe" "%cd%/oish_gen.exe*" /y >nul