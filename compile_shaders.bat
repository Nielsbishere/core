@echo off
call :treeProcess
goto :eof

rem Get the date as a YYYYMMDDhhmmss format
:getDate
set target=%~1
for /f "tokens=*" %%a in ('WMIC datafile WHERE name^="%target:\=\\%" get lastmodified') do ( 
	for /f "tokens=1 delims=." %%b in ("%%a") do (
		set lastMod=%%b
	)
) 
exit /b

rem Cut the string in half and compare which date is bigger
rem compareDates(a, b) = a > b = result
:compareDates
set t0=%1
set t1=%2
set /a target0=%t0:~0,8%
set /a target1=%t1:~0,8%
set /a target2=%t0:~8%
set /a target3=%t1:~8%

if %target0% geq %target1% (
	if NOT %target2% gtr %target3% (
		set result=0
	) else (
		set result=1
	)
) else (
	set result=0
)

exit /b

rem Figure out whether or not to build
:perShader

set /a end=%~2-1

rem If it doesn't exist, build it
if NOT exist "%~4.oiSH" goto build

rem If it is older than this batch or the oish_gen files
rem Build it

set shaderBase=%~4
call :getDate "%shaderBase%.oiSH"
set oiSH=%lastMod%

rem echo Checking source
for /l %%y in (0, 1, %end%) do (
	set "stage=%shaderBase%!stages[%%y]!"
	call :getDate !stage!
	call :compareDates !lastMod! %oiSH%
	rem echo Checking source (!stage! !lastMod! %oiSH% !oiSH!)
	
	if !result!==1 (
		goto :build
	)
)

rem echo Checking compiler script
call :getDate "%~dp0compile_shaders.bat"
call :compareDates %lastMod% %oiSH%
if %result%==1 (
	rem echo Compiler script updated
	goto :build
)

rem echo Checking shader compiler
call :getDate "%~dp0oish_gen.exe"
call :compareDates %lastMod% %oiSH%
if %result%==1 (
	rem echo Shader compiler updated
	goto :build
)
	
echo %~4.oiSH is up to date
exit /b
	
rem Build a shader
:build
	
rem Prepare the arguments

set "args="%~dp0oish_gen.exe" "%~4" "%~1" !stages[0]!"
for /l %%x in (1, 1, %end%) do set args=!args! !stages[%%x]!

rem generate the per stage spirv

for /l %%y in (0, 1, %end%) do (

  rem Compile shader source into spir-v code
  "%VULKAN_SDK%/Bin/glslangValidator.exe" -V -e main "%~4!stages[%%y]!" -o "%~4!stages[%%y]!.spv" >nul 2>&1
  
  rem Optimize shader source code and strip all reflection data from source
  "%VULKAN_SDK%/Bin/spirv-opt.exe" -Os -O "%~4!stages[%%y]!.spv" -o "%~4!stages[%%y]!.ospv"
  "%VULKAN_SDK%/Bin/spirv-remap.exe" --do-everything -i "%~4!stages[%%y]!.ospv" -o ../shaders/
  
)

rem Execute compilation to oiSH
cmd /c "%args%"

rem Remove all the .spv and .ospv files generated for this shader
rem Since those are intermediate files

for /l %%y in (0, 1, %end%) do (
 del /F /Q "%~4!stages[%%y]!.ospv"
 del /F /Q "%~4!stages[%%y]!.spv"
)

exit /b

:treeProcess

setlocal enabledelayedexpansion

set /a i=0
set /a j=-1

for /f "eol=: delims=" %%f in (
  'dir /b /a-d /one *.vert *.frag *.comp *.geom 2^>nul'
) do (
	
	rem Call 'perShader' if the shader name changes
	
	if NOT !name!==%%~nf (
			
		if NOT "!name!"=="" (
			call :perShader !name! !i! !j! !fullname!
		)
		
		set stages[0]=%%~xf
		set /a i=1
		set /a j=!j!+1
	)
	
	if !name!==%%~nf (
		set stages[!i!]=%%~xf
		set /a i=!i!+1
	)
	
	
	set name=%%~nf
	set fullname=%%~df%%~pf%%~nf
)

if defined name (
	call :perShader %name% %i% %j% %fullname%
)

endlocal

rem go through other directories inside this directory
for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)