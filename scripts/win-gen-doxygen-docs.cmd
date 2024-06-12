@echo off
setlocal enabledelayedexpansion
pushd %~dp0\..\docs
call ..\tools\bin\doxygen Doxyfile
endlocal
popd
pause
exit 0