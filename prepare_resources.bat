@echo off
cd "%~dp0app"
cmd /c "%~dp0compile_shaders"
cmd /c "%~dp0bake_models"
cd "%~dp0"