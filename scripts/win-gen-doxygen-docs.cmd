@echo off
setlocal enabledelayedexpansion
pushd %~dp0\..\docs

set "VT_VERSION="
for /f "usebackq delims=" %%i in ("..\VERSION") do (
	set "VT_VERSION=%%i"
	set "VT_VERSION=!VT_VERSION: =!"
	set "VT_VERSION=!VT_VERSION:	=!"
)
if not defined VT_VERSION (
	echo Error: VERSION file is empty or not found.
	exit /b 1
)

if not exist "src" (
	mkdir "src"
)

:: Injects table of contents into README
type "..\README.md" > "src\README.autogen.md"
echo. >> "src\README.autogen.md"
echo [TOC] >> "src\README.autogen.md"
echo. >> "src\README.autogen.md"

call ..\tools\bin\doxygen Doxyfile
endlocal
popd
pause
exit 0
