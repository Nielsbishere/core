@echo off
setlocal enabledelayedexpansion
call :treeProcess
endlocal
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
set t=%1
set tt=%2

set /a target0=1%t:~0,8%
set /a target1=1%tt:~0,8%
set /a target2=1%t:~8%
set /a target3=1%tt:~8%

set /a target0=%target0% %% 100000000
set /a target1=%target1% %% 100000000
set /a target2=%target2% %% 1000000
set /a target3=%target3% %% 1000000

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


rem go through other directories inside this directory
for /D %%d in (*) do (
    >NUL cd %%d
    call :treeProcess
    cd ..
)