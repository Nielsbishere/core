@echo off
cd res/models
call :treeProcess
cd ../../
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

if %target0% equ %target1% (
	if %target2% gtr %target3% (
		set result=1
	) else (
		set result=0
	)
) else (
	if %target0% gtr %target1% (
		set result=1
	) else (
		set result=0
	)
)

exit /b

:treeProcess

setlocal enabledelayedexpansion

set executable="%~dp0oirm_gen.exe"
set script="%~dp0bake_models.bat"

set /a i=0
set /a j=-1

call :getDate %executable%
set exeMod=%lastMod%

call :getDate %script%
set batMod=%lastMod%

for %%f in (*.obj *.fbx) do (
	
	set "trg=%%~df%%~pf%%~nf.oiRM"
	
	set /a final=0
	
	if exist "!trg!" (
	
		call :getDate "!trg!"
		set targetMod=!lastMod!
		call :getDate "%%~ff"
	 
		call :compareDates !lastMod! !targetMod!
		if !result!==1 (
			set /a final=1
		)
	 
		call :compareDates %exeMod% !targetMod!
		if !result!==1 (
			set /a final=1
		)
	 
		call :compareDates %batMod% !targetMod!
		if !result!==1 (
			set /a final=1
		)
	 
		if !final!==0 (
			echo !trg! is up to date
		)
		
	) else (
		set /a final=1
	)
	
	
	if !final!==1 (
		cmd /c "%executable% "%%~ff""
	)
)

endlocal



rem go through other directories inside this directory
for /D %%d in (*) do (
    cd %%d
	echo %%d
    call :treeProcess
    cd ..
)