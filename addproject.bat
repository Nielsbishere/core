@echo off

>NUL chcp 65001

if "%1"=="" goto :error
if "%2"=="" goto :error1

if exist %1 goto :error2
if not exist %2 goto :error1

set PROJECT_NAME=%1

mkdir "%1"
xcopy "%2" "%1" /s /e /h
cd %1
call :treeProcess
cd ..
set PROJECT_NAME=
goto :eof

:treeProcess
for %%f in (*.child) do (
	powershell -Command "(gc %%f) -replace 'PROJECT_NAME', '%PROJECT_NAME%' | Out-File %%f"
	powershell -Command "Get-Content %%f | Set-Content -Encoding utf8 %%~nf"
	del "%%f"*
)

for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)
exit /b

:error
	echo Please add a project name (no spaces and lowercase)
	echo Usage: addproject test {template}
	pause
	goto :eof
	
:error1
	echo Please add an existing template to inherrit from (no spaces and lowercase)
	echo Usage: addproject test {template}
	pause
	goto :eof
	
:error2
	echo That folder already exists
	pause
	goto :eof