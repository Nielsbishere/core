@echo off
cd "%~dp0app"
cmd /c "%~dp0oish_gen"
cmd /c "%~dp0oirm_gen"
cd "%~dp0"