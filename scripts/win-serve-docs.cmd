@echo off
setlocal enabledelayedexpansion
pushd %~dp0\..\docs
call ..\tools\bin\mdbook serve --open
endlocal
popd
pause
exit 0