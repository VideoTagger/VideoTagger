@echo off
pushd %~dp0\..\
set /p vs_ver="What is your Visual Studio version? (2019/2022): "
if %vs_ver% lss 2019 (
	echo Error: Unsupported Visual Studio version
	goto :Error
)
if %vs_ver% gtr 2022 (
	echo Error: Unsupported Visual Studio version
	goto :Error
)
call tools\premake\windows\premake5.exe vs%vs_ver%
if %errorlevel% neq 0 (
	echo Error: Premake failed to generate project files
	goto :error
)

set /p response="Would you like to open the solution file? (y/n): "

if /i "%response%"=="y" (
    for %%f in (*.sln) do (
		echo Opening %%f...
        start "" "%%f"
		goto :success
    )
	echo Error: No solution file found in the root directory
)
:success
	popd
	pause
	exit 0

:error
	popd
	pause
	exit -1