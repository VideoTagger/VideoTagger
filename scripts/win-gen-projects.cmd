@echo off
setlocal enabledelayedexpansion
pushd %~dp0\..\

call python scripts\setup.py
if %errorlevel% neq 0 (
	echo Error: Setup failed
	goto :error
)
call pip install -r scripts\requirements.txt
if %errorlevel% neq 0 (
	echo Error: Setup failed
	goto :error
)

call python scripts\gen_about.py
if %errorlevel% neq 0 (
	echo Error: Setup failed
	goto :error
)

if exist win-projects.buildcfg (
	set cfg_loaded=1
	set /p vs_ver=<win-projects.buildcfg
) else (
	set cfg_loaded=0
	set /p vs_ver="What is your Visual Studio version? [2019/2022]: "
)

set "vs_ver=%vs_ver: =%"

if %vs_ver% lss 2019 (
	echo Error: Unsupported Visual Studio version
	goto :error
)
if %vs_ver% gtr 2022 (
	echo Error: Unsupported Visual Studio version
	goto :error
)

call tools\bin\premake5.exe vs%vs_ver%
if %errorlevel% neq 0 (
	echo Error: Premake failed to generate project files
	goto :error
)

if %cfg_loaded% equ 0 (
	set /p save_cfg="Would you like to save this build config? [y/N]: "

	if /i "!save_cfg!"=="y" (
		echo Saving config...
		echo %vs_ver% > win-projects.buildcfg
	)
)

set /p response="Would you like to open the solution file? [y/N]: "

if /i "%response%"=="y" (
    for %%f in (*.sln) do (
		echo Opening %%f...
        start "" "%%f"
		endlocal
		popd
		exit 0
    )
	echo Error: No solution file found in the root directory
)

:success
	endlocal
	popd
	pause
	exit 0

:error
	endlocal
	popd
	pause
	exit -1