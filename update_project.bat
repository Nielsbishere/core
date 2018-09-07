@echo off

xcopy "%~dp0clean.bat" "%cd%/clean.bat*" /y >nul
xcopy "%~dp0reload.bat" "%cd%/reload.bat*" /y >nul
xcopy "%~dp0compile_shaders.bat" "%cd%/compile_shaders.bat*" /y >nul
xcopy "%~dp0bake_models.bat" "%cd%/bake_models.bat*" /y >nul
xcopy "%~dp0prepare_resources.bat" "%cd%/prepare_resources.bat*" /y >nul
xcopy "%~dp0make_android.bat" "%cd%/make_android.bat*" /y >nul
xcopy "%~dp0make_pc.bat" "%cd%/make_pc.bat*" /y >nul
xcopy "%~dp0oirm_gen.exe" "%cd%/oirm_gen.exe*" /y >nul
xcopy "%~dp0oish_gen.exe" "%cd%/oish_gen.exe*" /y >nul